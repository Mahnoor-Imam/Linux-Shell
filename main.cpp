#include<iostream>
#include"myShell.h"
#include <sys/wait.h>
#include<stdlib.h>

using namespace std;

int main(int argc,char **argv)
{	
	system("clear");
	char  command_Line[100];
	command cmd;
	cmd.name = NULL;

	char *pathv[64];
	parse_Path(pathv);

	int exec_flag;
	
	cout<<"        ******************   MY SHELL   ******************"<<endl;
	
	while(true)
	{
		print_Prompt();
		
		read_Command(command_Line);

				
		exec_flag = process_Command(command_Line , &cmd);

		if(exec_flag == 1)
		{
			if(exec_redirected_Command(command_Line , &cmd,pathv)==false)
			{
				parse_Command(command_Line , &cmd);
/*cout<<cmd.argc<<endl;
for(int i=0;i<cmd.argc;i++)
	cout<<cmd.argv[i]<<endl;*/
				if(exec_built_in_Command(&cmd) == false)// no looking up path for built in commands
				{
					cmd.name = look_Up_Path(cmd.argv , pathv);
					exec_Command(&cmd);
				}
			}
		}
		else if(exec_flag == 2)
		{
			exec_piped_Command(&cmd,pathv);
			//cout<<"done"<<endl;
		}
	}
		
	return 0;
}
