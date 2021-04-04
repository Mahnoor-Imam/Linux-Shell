#include<iostream>

#define LINE_LEN 80
#define MAX_ARGS 64
#define MAX_ARG_LEN 16
#define MAX_PATHS 64
#define MAX_PATH_LEN 96

using namespace std;


struct command
{
	char *name;
	int argc;   //argument count
	char *argv[MAX_ARGS]; //argument array
	int argc_P;   //piped argument count
	char *argv_P[MAX_ARGS]; //piped argument array

	char *input_file;
	char *output_file;
};


void print_Prompt();
void read_Command(char * buffer);
void parse_Command(char *str , command *cmd);
void parse_Path(char *dirs[]);
char* look_Up_Path(char ** argv , char ** dir );
void exec_Command(command *cmd);
bool exec_built_in_Command(command *cmd);
void exec_piped_Command(command *cmd,char **pathv);
bool check_for_pipe(char *str , command *cmd);
int process_Command(char *str , command *cmd);
bool check_for_input_redirect(char *str , command *cmd);
bool check_for_output_redirect(char *str , command *cmd);
bool exec_redirected_Command(char *str ,command *cmd,char **pathv);
