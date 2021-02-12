#include "get_path.h"
int pid;
int sh( int argc, char **argv, char **envp);
char *which(char *command, struct pathelement *pathlist);
char *where(char *command, struct pathelement *pathlist);
void list ( char *dir );
void printenv(char **envp);
void print_working_directory();
void listCommand(char *directory);
void processID();
void change_directory(char *directory, char *prevDir, char *homeDir, char *pwd);
void killProgram(char *sig, char *process);
char* promptFunc(char *promptInput);
void printenvironment(int arg_amount, char **environ, char *env_arg);
void setenvironment(char **argument);
void delete_list(struct pathelement *first);
int stringChecker(char *char_to_check, char *invalid_characters);
#define PROMPTMAX 32
#define MAXARGS 10