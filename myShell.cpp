#include <iostream>
#include "myShell.h"
#include <string.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <fcntl.h>

using namespace std;

void print_Prompt()
{
	string prompt=":~$ myShell>> ";
	char buffer[100];
	getcwd(buffer,sizeof(buffer)); //getting current directory
	cout<<buffer<<prompt;
}

void read_Command(char * buffer)
{
	cin.getline(buffer , 100); //getting input from user
}

void parse_Command(char *str , command *cmd) //parse the command and tokenize for arguments
{
	char *token;
	token = strtok(str," ");
	cmd->argc=0;
	int i=0;
	if(token!=NULL)
	{
		cmd->argv[i]=token;
		i++;
		while(token!=NULL)
		{
			token=strtok(NULL," ");
			cmd->argv[i]= token;
			i++;
			cmd->argc++;
		}
	}
	i++;
	cmd->argv[i]=NULL;
}

void parse_Path(char *dirs[]) //parse the path for tokenizing directories
{
	char*path_Env;
	char*path;
	for(int i=0; i<MAX_ARGS; i++)
		dirs[i]=NULL;
	path_Env=(char*)getenv("PATH");
	path=(char*)malloc(strlen(path_Env)+1);
	strcpy(path, path_Env);
	char*token;
	token=strtok(path,":");
	int i=0;
	if(token!=NULL)
	{
		dirs[i]=token;
		i++;
		while(token!=NULL)
		{
			token=strtok(NULL,":");
			dirs[i]=token;
			i++;
		}
	}
i++;
dirs[i]=NULL;
}

char* look_Up_Path(char **argv , char **dir)
{	
	char* result_path = new char [50];
	int i = 0;

	if(*argv[0] == '/')
	{

		int permission = access(argv[0] , F_OK); //checking existance only
		if(permission == 0)
			return argv[0];
		else
			return NULL;
	}
	for(i; i<MAX_PATHS; i++)
	{	
		strcpy(result_path , dir[i]);
		strcat(result_path , "/");
		strcat(result_path , argv[0]);
		int permission = access(result_path, F_OK|X_OK);

		if(permission == 0)
		{
			return result_path;
			
		}
	} 
	cout<<"File Name not found in any path variable "<<endl;
	return NULL;
}

//Function to execute simple commands
void exec_Command(command *cmd)
{
	if(cmd->name == NULL)
	{
		cout<<"ERROR!! NOT A SUITABLE COMMAND NAME"<<endl;
		return;
	}
	else
	{
		int pid = fork();
		if(pid == -1)
		{
			cout<<"Failed to fork a child process"<<endl;
			return;
		}
		if(pid == 0)
		{
			execv(cmd->name , cmd->argv);
			cout<<"Command not Found"<<endl;
		}
		if(pid > 0)
		{
			wait(NULL);
			return;
		}
	}
}

//Function to execute built-in commands
bool exec_built_in_Command(command *cmd)
{
	int num_of_built_in_cmds=2;
	char **built_in_cmds_list=new char*[num_of_built_in_cmds];
	built_in_cmds_list[0] = "exit";
	built_in_cmds_list[1] = "cd";
	
	//comparing with argv[0] as their will be no name returned to built in commands in look_Up_Path function in main
	if(strcmp(cmd->argv[0],built_in_cmds_list[0]) == 0)
	{
		//cout<<"exiting"<<endl;
		cout<<"        ******************   MY SHELL   ******************"<<endl;
		exit(0);
		return true;
	}
	if(strcmp(cmd->argv[0],built_in_cmds_list[1]) == 0)
	{
		chdir(cmd->argv[1]);
		char buffer[100];
		getcwd(buffer,sizeof(buffer)); //getting current directory
		cout<<endl<<"current directory changed to "<<buffer<<endl;
		return true;
	}
	return false;
}

//Function that checks for pipes and tokenizes for the piped arguments accordingly
bool check_for_pipe(char *str , command *cmd)
{
	char *token;
	token = strtok(str,"|");
	cmd->argc_P=0;
	int i=0;
	if(token!=NULL)
	{
		cmd->argv_P[i]=token;
		i++;
		while(token!=NULL)
		{
			token=strtok(NULL,"|"); //added pipe sign
			cmd->argv_P[i]= token;
			i++;
			cmd->argc_P++;
		}
	}
	i++;
	cmd->argv_P[i]=NULL;
	if(cmd->argv_P[1] == NULL)
		return false;  //no pipe found
	return true;
}

//Function that checks whether it is a piped or simple command
int process_Command(char *str , command *cmd)
{
	bool piped = check_for_pipe(str , cmd);
	if(piped == true)
	{
		return 2;
	}
	else
	{
		return 1;
	}
}

void exec_piped_Command(command *cmd,char **pathv)
{
/*cout<<cmd->argc_P<<endl;
for(int i=0;i<cmd->argc_P;i++)
	cout<<cmd->argv_P[i]<<endl;*/

	int in = dup(0);
	int out = dup(1);
	
//cout<<"STD_IN :"<<in<<endl<<"STD_OUT :"<<out<<endl;

int loop=cmd->argc_P;

// making required number of pipes
int **pp = new int *[loop -1];
for(int i=0;i<loop-1;i++)
{
	pp[i]=new int[2];
	pipe(pp[i]);
}
//
	//making child for each piped argument
	for(int i=0;i<loop;i++)
	{
		int pid=fork();
		if(i==0)
		{
			if(pid==0)
			{
				dup2(pp[i][1],1);
				close(pp[i][1]); // must
				close(pp[i][0]); //
			//cout<<"1st out redirect at :"<<dup(1)<<endl;
			bool check_infile=check_for_input_redirect(cmd->argv_P[i], cmd);
				if(check_infile==false)
				{
					parse_Command(cmd->argv_P[i] , cmd);
					cmd->name = look_Up_Path(cmd->argv , pathv);
					execv(cmd->name , cmd->argv);
					cout<<"Command not Found"<<endl;
				}
				else
				{
					int fd=open(cmd->input_file,O_RDONLY);
	//cout<<"in file :"<<fd<<endl;
				dup2(fd,0);
				close(fd);
	//cout<<dup(0);
				parse_Command(cmd->argv[0] , cmd);
				cmd->name = look_Up_Path(cmd->argv , pathv);
				execv(cmd->name , cmd->argv);
				}
			}
		}
		else if(i==loop-1)
		{
			if(pid==0)
			{
				dup2(pp[i-1][0],0);
				close(pp[i-1][1]); // must
				close(pp[i-1][0]); //
			//cout<<"1st in redirect at :"<<dup(0)<<endl;
			bool check_outfile=check_for_output_redirect(cmd->argv_P[i], cmd);
				if(check_outfile==false)
				{
					parse_Command(cmd->argv_P[i] , cmd);
					cmd->name = look_Up_Path(cmd->argv , pathv);
					execv(cmd->name , cmd->argv);
					cout<<"Command not Found"<<endl;
				}
				else
				{
					int fd=open(cmd->output_file,O_WRONLY);
	//cout<<"out file :"<<fd<<endl;
				dup2(fd,1);
				close(fd);
	//cout<<dup(1);
				parse_Command(cmd->argv[0] , cmd);
				cmd->name = look_Up_Path(cmd->argv , pathv);
				execv(cmd->name , cmd->argv);
				}
			}
		}
		else
		{
			if(pid==0)
			{
				dup2(pp[i][1],1);
				close(pp[i][1]); // must
				close(pp[i][0]); //				
				dup2(pp[i-1][0],0);
				close(pp[i-1][1]); // must
				close(pp[i-1][0]); //
			//cout<<"1st in redirect at :"<<dup(0)<<endl;
			//cout<<"1st out redirect at :"<<dup(1)<<endl;
				parse_Command(cmd->argv_P[i] , cmd);
				cmd->name = look_Up_Path(cmd->argv , pathv);
				execv(cmd->name , cmd->argv);
				cout<<"Command not Found"<<endl;
			}
		}
		if(i!=loop-1)
			close(pp[i][1]); //must
		wait(NULL);
	}
}

//Function that checks for input redirection and finds input file name
bool check_for_input_redirect(char *str , command *cmd)
{
	int flag=0;
	int j=0;
	while(str[j]!='\0')
	{
		if(str[j]=='<')
			flag=1;
		j++;
	}
	if(flag==0)
		return false;  //if there is no input redirection
	
	char *token;
	token = strtok(str,"<");
	cmd->argc=0;
	int i=0;
	int flag_in=0;
	if(token!=NULL)
	{
		cmd->argv[i]=token;
		i++;
		while(token!=NULL)
		{
			token=strtok(NULL," ");
			if(flag_in==0)
			{
				cmd->input_file=token;
				flag_in=1;
			}
			else
			{
				cmd->argv[i]= token;
				cmd->argc++;
			}
			i++;
			
		}
	}
	i++;
	cmd->argv[i]=NULL;
	return true;
}

//Function that checks for output redirection and finds output file name
bool check_for_output_redirect(char *str , command *cmd)
{
	int flag=0;
	int j=0;
	while(str[j]!='\0')
	{
		if(str[j]=='>')
			flag=1;
		j++;
	}
	if(flag==0)
		return false; //if there is no output redirection

	char *token;
	token = strtok(str,">");
	cmd->argc=0;
	int i=0;
	int flag_out=0;
	if(token!=NULL)
	{
		cmd->argv[i]=token;
		i++;
		while(token!=NULL)
		{
			token=strtok(NULL," ");
			if(flag_out==0)
			{
				cmd->output_file=token;
				flag_out=1;
			}
			else
			{
				cmd->argv[i]= token;
				cmd->argc++;
			}
			i++;
			
		}
	}
	i++;
	cmd->argv[i]=NULL;
	return true;
}

//Function to execute a command with redirection
bool exec_redirected_Command(char *str ,command *cmd,char **pathv)
{

	bool check_outfile=check_for_output_redirect(str, cmd);
	bool check_infile=check_for_input_redirect(str , cmd);  //this order matters in checking if it's case 3

	if(check_infile==true && check_outfile == false) //case 1 : only input redirected
	{
//cout<<"INPUT CASE ONLY"<<endl;
		int fd;
		int pid=fork();
		if(pid <0)
			cout<<"failed"<<endl;
		else if(pid==0)
		{
			fd=open(cmd->input_file,O_RDONLY);
//cout<<"in file :"<<fd<<endl;
			dup2(fd,0);
			close(fd);
//cout<<dup(0);
			parse_Command(cmd->argv[0] , cmd);
			cmd->name = look_Up_Path(cmd->argv , pathv);
			execv(cmd->name , cmd->argv);
			
		}
		wait(NULL);
		return true;
	}
	
	if(check_outfile==true && check_infile == false) //case 2 : only output redirected
	{
//cout<<"OUTPUT CASE ONLY"<<endl;
		int fd;
		int pid=fork();
		if(pid <0)
			cout<<"failed"<<endl;
		else if(pid==0)
		{
			fd=open(cmd->output_file,O_WRONLY);
//cout<<"out file :"<<fd<<endl;
			dup2(fd,1);
			close(fd);
//cout<<dup(1);
			parse_Command(cmd->argv[0] , cmd);
			cmd->name = look_Up_Path(cmd->argv , pathv);
			execv(cmd->name , cmd->argv);	
		}
		wait(NULL);
		return true;
	}

	if(check_outfile==true && check_infile==true) //case 3 : I/O both are redirected
	{
//cout<<"BOTH INPUT AND OUTPUT CASE"<<endl;
		check_for_output_redirect(str, cmd);           //to get
		check_for_input_redirect(cmd->argv[0], cmd);     //file names
		int fd_o,fd_i;
		int pid=fork();
		if(pid <0)
			cout<<"failed"<<endl;
		else if(pid==0)
		{
			fd_i=open(cmd->input_file,O_RDONLY);
			fd_o=open(cmd->output_file,O_WRONLY);
//cout<<"in file :"<<fd_i<<endl;
//cout<<"out file :"<<fd_o<<endl;
			dup2(fd_o,1);
			dup2(fd_i,0);
			close(fd_i);
			close(fd_o);
//cout<<dup(0)<<endl<<dup(1)<<endl;
			parse_Command(cmd->argv[0] , cmd);
			cmd->name = look_Up_Path(cmd->argv , pathv);
			execv(cmd->name , cmd->argv);
			
		}
		wait(NULL);
		return true;
	}

	return false; // if command is not I/O redirected type
}
