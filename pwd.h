/* Name: Ahmad Tamimi
   Professor: Greg Silber
   Date: 10/15/2019
   This program is a custom shell with basic built in commands
   It is built on top of bash. It has some pre built-in functions, 
   but it can run other executable programs in the system (like ls, file, etc.)
   Running the program gives the user a very similar experience to bash.
   You can do anything in this shell that you can do with bash!
*/

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <wordexp.h>
#include "sh.h"
#define BUFFERSIZE 1024
int procRun = 0;

int sh(int argc, char **argv, char **envp)
{
  char *prompt = calloc(PROMPTMAX, sizeof(char));
  char *commandline = calloc(MAX_CANON, sizeof(char));
  char *command, *arg, *commandpath, *p, *pwd, *owd;
  char **args = calloc(MAXARGS, sizeof(char *));
  int uid, i, status, argsct, go = 1;
  struct passwd *password_entry;
  char *homedir;
  struct pathelement *pathlist;
  char *prevWorkDir = malloc(1024 * sizeof(char));
  //prevWorkDir = "";

  int firstCD = 1;
  uid = getuid();
  password_entry = getpwuid(uid);   /* get passwd info */
  homedir = password_entry->pw_dir; /* Home directory to start
						  out with*/

  if ((pwd = getcwd(NULL, PATH_MAX + 1)) == NULL)
  {
    perror("getcwd");
    exit(2);
  }
  owd = calloc(strlen(pwd) + 1, sizeof(char));
  memcpy(owd, pwd, strlen(pwd));
  memcpy(prevWorkDir, pwd, strlen(pwd));
  prompt[0] = ' ';
  prompt[1] = '\0';

  /* Put PATH into a linked list */
  pathlist = get_path();

  while (go)
  {
    /*handle ctrl Z and C signals*/
    signal(SIGINT, sig_handler);
    signal(SIGTSTP, sig_handler);
    signal(SIGTERM, sig_handler);

    /* print your prompt */
    char actualWorkingDir[1024];
    int wildFlag = 0;
    getcwd(actualWorkingDir, sizeof(actualWorkingDir));
    printf("%s [%s]> ", prompt, actualWorkingDir);
    /* get command line and process */
    char buffer[1024];
    strcpy(buffer, "\0");
    char temp[1024];
    char **cmds;
    int cnt = 1;
    fgets(buffer, 1024, stdin);
    char* isEnter = buffer;
    if(!strcmp(isEnter, "\n")){
      //printf("\ngot enter \n");
      continue;
    }
      strtok(buffer, "\n");
      strcpy(temp, buffer);
      //strtok(temp, " ");
      //strtok(temp, " ");
      while (strtok(NULL, " "))
      {
        cnt++;
      }
      cmds = calloc(MAXARGS, sizeof(char *));
      cmds[cnt] = 0;
      char *s;
      s = strtok(buffer, " ");
      cnt = 0;
      while (s)
      {
        cmds[cnt] = malloc(strlen(s) + 1);
        strcpy(cmds[cnt++], s);
        s = strtok(NULL, " ");
      }
      free(s);
      for(int i = 0; i < cnt; i++){
        strtok(cmds[i], "\n");
      }
      //Ignore Ctrl+D
      if (cmds[0] == NULL)
      {
        printf("\nCannot Ctrl-D \n");
        free(cmds[0]);
        free(cmds);
        continue;
      }
      //check if built in function
      //If command is exit, free up memory, and ready the program to exit the shell
      else if (!strcmp(cmds[0], "exit"))
      {
        printf("exiting the shell... \n");
        free(prompt);
        freePathlist(pathlist);
        free(commandline);
        free(args);
        free(prevWorkDir);
        for (int i = 0; i < MAXARGS; i++)
        {
          free(cmds[i]);
        }
        free(cmds);
        free(pwd);
        free(owd);
        fflush(stdout);
        exit(0);
      }
      //pwd: built in function, print out the current working directory
      else if (!strcmp(cmds[0], "pwd"))
      { 
        printf("Executing built in %s command \n", cmds[0]);
        char currWorkDir[1024];
        getcwd(currWorkDir, sizeof(currWorkDir));
        printf("%s\n", currWorkDir);
      }
      
      //list: calls the list built in function. It lists out
      //the contents of a specificed directory. With no arguments, lists the files in the current 
      //working directory one per line. With arguments, lists the files in each directory given as
      //an argument, with a blank line then the name of the directory followed by a : before the list 
      //of files in that directory.
      else if (!strcmp(cmds[0], "list"))
      { 
        printf("Executing built in %s command \n", cmds[0]);
        if (cnt == 1)
        {
          char currWorkDir[1024];
          getcwd(currWorkDir, sizeof(currWorkDir));
          printf("%s: \n", currWorkDir);
          list(currWorkDir);
        } 
        else
        { //print out the list for more than one directory
          int k = 1;
          for (k; k < cnt; k++)
          {
            printf("%s: \n", cmds[k]);
            list(cmds[k]);
          }
        }
      }

      //which: calls the which built in function to locate
      // the path to a function (or NUll if no path exists)
      else if (strcmp(cmds[0], "which") == 0)
      {
        printf("Executing built in %s command \n", cmds[0]);
        if (cnt == 1)
        {
          printf("Incorrect number of arguments for %s function. \n", cmds[0]);
        }
        else
        {
          int i = 1;
          char *comPassed; // = malloc(1024 * sizeof(char));
          for (i; i < cnt; i++)
          {
            comPassed = strdup(cmds[i]);
            char *comPath = which(comPassed, pathlist);
            free(comPassed);
            if (comPath != NULL)
            {
              printf("%s \n", comPath);
              free(comPath);
            }
            else
            {
              printf("%s command cannot be found. \n", cmds[1]);
            }
          }
        }
      }

      //where: calls the where built in function to locate
      //the path to a function (or NUll if no path exists)
      else if (strcmp(cmds[0], "where") == 0)
      {
        printf("Executing built in %s command \n", cmds[0]);
        if (cnt == 1)
        {
          printf("Incorrect number of arguments for %s function. \n", cmds[0]);
        }
        else
        {
          int j = 1;
          char *comPassed; // = malloc(1024 * sizeof(char));
          for (j; j < cnt; j++)
          {
            comPassed = strdup(cmds[j]);
            char *comPath = where(comPassed, pathlist);
            free(comPassed);
            if (comPath != NULL)
            {
              printf("%s \n", comPath);
              free(comPath);
            }
            else
            {
              printf("%s command cannot be found. \n", cmds[1]);
            }
          }
        }
      }

      //cd: calls the cd built in function to change the directory to
      //the specified one by the user and it can accept arguments
      else if (strcmp(cmds[0], "cd") == 0)
      {
        printf("Executing built in %s command \n", cmds[0]);
        char currWorkDir[1024];
        getcwd(currWorkDir, sizeof(currWorkDir));
        DIR *direct;
        struct dirent *entryp;
        if (firstCD == 1)
        {
          strcpy(prevWorkDir, currWorkDir);
          firstCD = 0;
        }
        if (cnt == 2)
        {
          direct = opendir(cmds[1]);
        }
        if (cnt == 1)
        {
          strcpy(prevWorkDir, currWorkDir);
          cd(getenv("HOME"), prevWorkDir);
        }
        else if (cnt == 2)
        {
          char *tmpPrev = malloc(1024 * sizeof(char));
          strcpy(tmpPrev, prevWorkDir);
          if (!strcmp(cmds[1], "-"))
          {
            strcpy(prevWorkDir, currWorkDir);
          }
          else if (direct)
          {
            strcpy(prevWorkDir, currWorkDir);
          }
          else if (!strcmp(cmds[1], ".."))
          {
            strcpy(prevWorkDir, currWorkDir);
          }
          closedir(direct);
          cd(cmds[1], tmpPrev);
          free(tmpPrev);
        }
        else
        {
          printf("Incorrect number of arguments for %s command, the number of arguments should be 1\n", cmds[0]);
        }
      }

      //prompt: cit is a built in function. When ran with no arguments, 
      //prompts for a new prompt prefix string. 
      //When given an argument make that the new prompt prefix.
      else if (strcmp(cmds[0], "prompt") == 0)
      { 
        printf("Executing built in %s command \n", cmds[0]);
        if (cnt == 1)
        {
          char promptDisplayed[1024];
          printf("input the prompt prefix: ");
          fgets(promptDisplayed, 1024, stdin);
          strtok(promptDisplayed, "\n");
          strcpy(prompt, promptDisplayed);
        }
        else if (cnt == 2)
        {
          strcpy(prompt, cmds[1]);
        }
        else
        {
          printf("Incorrect number of arguments for %s method. \n", cmds[0]);
        }
      }

      //pid: built in command prints out the process ID of the shell
      else if (strcmp(cmds[0], "pid") == 0)
      {
        printf("Executing built in %s command \n", cmds[0]);
        if (cnt > 1)
        {
          printf("Incorrect number of arguments for %s function", cmds[0]);
        }
        else
        {
          printf("Process ID: %d \n", getpid());
        }
      }

      //kill: built in command, kills a specified process(PID), and it can send a specified signal to a process(PID)
      //When given just a pid sends a SIGTERM to i. When given a signal number (with a - in front of it) sends that 
      //signal to the pid. (e.g., kill 5678, kill -9 5678).
      else if (strcmp(cmds[0], "kill") == 0)
      {
        printf("Executing built in %s command \n", cmds[0]);
        if (cnt == 1)
        {
          char pidKill[1024];
          printf("Enter the PID of the command to kill: ");
          fgets(pidKill, 1024, stdin);
          strtok(pidKill, "\n");
          kill(atoi(pidKill), SIGTERM);
        }
        else if (cnt == 2)
        {
          kill(atoi(cmds[1]), SIGTERM);
        }
        else if (cnt == 3)
        {
          if (strstr(cmds[1], "-") != NULL)
          {
            kill(atoi(cmds[2]), -1 * atoi(cmds[1]));
          }
        }
      }

      //printenv: built in command, it prints out the whole envinronment path
      else if (strcmp(cmds[0], "printenv") == 0)
      {
        printf("Executing built in %s command \n", cmds[0]);
        if (cnt == 1)
        {
          printenv(envp);
        }
        else if (cnt == 2)
        {
          if (getenv(cmds[1]) != NULL)
          {
            printf("%s\n", getenv(cmds[1]));
          }
          else
          {
            printf("Environment does not exist.\n");
          }
        }
        else
        {
          perror("setenv");
          fprintf(stderr, "too many arguments");
          printf("Incorrect number of arguments for %s function\n", cmds[0]);
        }
      }

      //setenv: built in command, if no arguments passed it behaves exactly 
      //like printenv: prints out the whole envinronment path
      // When ran with one argument, call getenv(3) on it. When 
      //called with two or more args print the same error message to 
      //stderr that tcsh does.

      else if (strcmp(cmds[0], "setenv") == 0)
      {
        printf("Executing built in %s command \n", cmds[0]);
        if (cnt == 1)
        {
          printenv(envp);
        }
        else if (cnt == 2)
        {
          setenv(cmds[1], " ", 1);
          if (strcmp(cmds[1], "PATH") == 0)
          {
            freePathlist(pathlist);
            pathlist = get_path();
          }
        }
        else if (cnt == 3)
        {
          setenv(cmds[1], cmds[2], 1);
          if (strcmp(cmds[1], "PATH") == 0)
          {
            freePathlist(pathlist);
            pathlist = get_path();
          }
        }
        else
        {
          perror("setenv");
          fprintf(stderr, "too many arguments");
          printf("Incorrect number of arguments for %s function\n", cmds[0]);
        }
      }

      //If absolute paths or ./ or ../ are entered by the user
      else if ((cnt >= 1) && (((cmds[0][0]) == '/') || (cmds[0][0] == '.')))
      {
        //If ./ is entered by user
        if ((cmds[0][0] == '.') && (cmds[0][1] == '/'))
        {
          char pathToExec[PATH_MAX];
          char currWorkDir[PATH_MAX + 1];
          getcwd(currWorkDir, sizeof(currWorkDir));
          sprintf(pathToExec, "%s/%s", currWorkDir, &cmds[0][2]);
          printf("Executing %s\n", pathToExec);
          if ((access(pathToExec, X_OK) == 0) && (!isDirectory(pathToExec)) && (isExecutable(pathToExec)))
          {
            int status = 0;
            pid_t pid1 = fork();
            if (pid1 == 0)
            {
              procRun = 1;
              execve(pathToExec, cmds, envp);
              exit(0);
            }
            else
            {
              procRun = -1;
              waitpid(pid1, &status, 0);
            }
          }
          else
          {
            printf("%s cannot be accessed/executed, or does not exist\n", &cmds[0][2]);
          }
        }

        //If ../ is entered by user
        else if ((cmds[0][0] == '.') && (cmds[0][1] == '.') && (cmds[0][2] == '/'))
        {
          char pathToExec[PATH_MAX];
          char currWorkDir[PATH_MAX + 1];
          getcwd(currWorkDir, sizeof(currWorkDir));
          sprintf(pathToExec, "%s/../%s", currWorkDir, &cmds[0][3]);
          printf("Executing %s\n", pathToExec);
          if ((access(pathToExec, X_OK) == 0) && (!isDirectory(pathToExec)) && (isExecutable(pathToExec)))
          {
            int status = 0;
            pid_t pid2 = fork();
            if (pid2 == 0)
            {
              procRun = 1;
              execve(pathToExec, cmds, envp);
              exit(0);
            }
            else
            {
              procRun = -1;
              waitpid(pid2, &status, 0);
            }
          }
          else
          {
            printf("%s cannot be accessed/executed\n", &cmds[0][3]);
          }
        }
        //If an absolute path is entered by the user
        else
        {
          printf("Executing %s\n", cmds[0]);
          if ((access(cmds[0], X_OK) != -1) && (access(cmds[0], F_OK) != -1) && (!isDirectory(cmds[0])) && (isExecutable(cmds[0]))) 
          {
            int status = 0;
            pid_t pid3 = fork();
            if (pid3 == 0)
            {
              for (int i = 0; i < cnt; i++)
              {
                printf("%s ", cmds[i]);
              }
              procRun = 1;
              execve(cmds[0], cmds, envp);
              exit(0);
            }
            else
            {
              procRun = -1;
              waitpid(pid3, &status, 0);
            }
          }
          else
          {
            printf("%s cannot be accessed/executed\n", cmds[0]);
          }
        }
      }
      /* else program to execute non-built in functions */
      else
      {
        pid_t pid;
        fflush(stdout);

        /* Check if wildcards exist in first or second argument*/
        /* If they do exist increment the count of cmds accordingly */

        if ((cnt >= 2) && (wildFlag == 0))
        {
          if (strchr(cmds[1], '*') || strchr(cmds[1], '?'))
          {
            cnt += wildcardHandler(cmds, 1);
            wildFlag = 1;
            if (strchr(cmds[1], '*') && strchr(cmds[1], '?'))
            {
              cnt += wildcardHandler(cmds, 1);
            }
          }
        }
        if ((cnt >= 3) && (wildFlag == 0))
        {
          if (strchr(cmds[2], '*') || strchr(cmds[2], '?'))
          {
            cnt += wildcardHandler(cmds, 2);
            wildFlag = 1;
            if (strchr(cmds[2], '*') && strchr(cmds[2], '?'))
            {
              cnt += wildcardHandler(cmds, 2);
            }
          }
        }
        
        /* find the process */
        
        char *process = which(cmds[0], pathlist);
        if (process == NULL)
        {
          printf("Command %s not found\n", cmds[0]);
        }
        else if (process != NULL)
        {
          /* do fork(), execve() and waitpid() */

          if ((pid = fork()) == 0)
          {
            procRun = 1;
            int status = execve(process, cmds, envp);
            if (status != -1)
            {
              printf("Executing: %s \n", cmds[0]);
            }
            else
            {
              printf("Execution failed %s\n", cmds[0]);
              exit(EXIT_FAILURE);
            }
            exit(0);
          }
          else if (pid > 0)
          {
            int childProcessStat = 0;
            procRun = -1;
            waitpid(pid, &childProcessStat, 0);
            free(process);
          }
          else
          {
            perror("Command not found");
          }
          //exit(0);
        }
      }
      procRun = 0;
      for (int i = 0; i < MAXARGS; i++)
      {
        free(cmds[i]);
      }
      free(cmds);
    }
  return 0;
} /* sh() */

//This function takes two parameters, a char* that repersents a command,
//and a pathlist. The function loops through the pathlist trying to find 
//the command. It then returns the location of the command as a path.
//If the command is not found a NULL is returned.

char *which(char *command, struct pathelement *pathlist)
{
  //* loop through pathlist until finding command and return it.  Return NULL when not found. */
  while (pathlist != NULL)
  {
    char *location;
    char tempVar[1024];
    strcpy(tempVar, pathlist->element);
    strcat(tempVar, "/");
    strcat(tempVar, command);
    if (access(tempVar, F_OK) != -1)
    {
      location = calloc(strlen(tempVar) + 1, sizeof(char));
      memcpy(location, tempVar, strlen(tempVar));
      return location;
    }
    else
    {
      pathlist = pathlist->next;
    }
  }
  return NULL;

} /* which() */

//This function takes two parameters, a char* that repersents a command,
//and a pathlist. The function loops through the pathlist trying to find 
//the command. It then returns the location of the command as a path.
//If the command is not found a NULL is returned.

char *where(char *command, struct pathelement *pathlist)
{
  /* similarly loop through finding all locations of command */
  while (pathlist != NULL)
  {
    char *location;
    char tempVar[1024];
    strcpy(tempVar, pathlist->element);
    strcat(tempVar, "/");
    strcat(tempVar, command);
    if (access(tempVar, F_OK) != -1)
    {
      location = calloc(strlen(tempVar) + 1, sizeof(char));
      memcpy(location, tempVar, strlen(tempVar));
      return location;
    }
    else
    {
      pathlist = pathlist->next;
    }
  }
  return NULL;

} /* where() */

//This is the signal handler (moved from main to here).
//It handles multiple signals(Ctrl+Z, Ctrl+C) and EOF (Ctrl+D)
//It only takes a signalNum as an int for the representation 
//of the signal to be handled
void sig_handler(int signalNum)
{
  if(signalNum == SIGINT && procRun == 0){
    signal(SIGINT, sig_handler);
    printf("\nShell Cannot be terminated by Ctrl+C");
    char actualWorkingDir[1024];
    getcwd(actualWorkingDir, sizeof(actualWorkingDir));
    printf("\n  [%s]> ", actualWorkingDir);
  }
  else if(signalNum == SIGTSTP && procRun == 0){
    signal(SIGTSTP, sig_handler);
    printf("\nCannot Ctrl+Z to exit the shell");
    char actualWorkingDir[1024];
    getcwd(actualWorkingDir, sizeof(actualWorkingDir));
    printf("\n  [%s]> ", actualWorkingDir);
  }
  fflush(stdout);
  /* define your signal handler */
}


//List: built in function, takes only one char arguement to list out it's contents
//It lists the files in the current working directory one per line.
//Does not return anything.

void list(char *dir)
{
  DIR *dr;
  struct dirent *entryp;
  dr = opendir(dir);
  if (dr == NULL)
  {
    printf("Cannot open directory %s\n", dir);
  }
  else
  {
    while ((entryp = readdir(dr)) != NULL)
    {
      printf("%s\n", entryp->d_name);
    }
    closedir(dr);
  }
}

//cd: built in function, takes two char * arguments, one is for the path to go to, the other is the previous path.
//It chdir(2) to directory given; with no arguments, chdir to the home directory, with a '-' as the only argument, chdirs 
//to directory previously in, the same as tcsh does.

void cd(char *path, char *prevPath)
{
  char *myPath = malloc(1024 * sizeof(char));
  strcpy(myPath, path);
  char currWorkDir[1024];
  getcwd(currWorkDir, sizeof(currWorkDir));

  if (strcmp(myPath, "..") == 0)
  {
    char parentDir[1024];
    snprintf(parentDir, sizeof(parentDir), "%s/..", currWorkDir);
    chdir(parentDir); 
    free(myPath);
    return; //returns nothing just leaves the function and does nothing after this part
  }
  if (!strcmp(myPath, "-"))
  {
    chdir(prevPath);
    printf("%s\n", prevPath);
    free(myPath);
    return;
  }
  if (strstr(myPath, "/"))
  { //checks to see if there is a path passed as an argument or not (assumed to be complete)
    if (chdir(path) == 0)
    {
      chdir(path);
    }
    else{
      printf("%s cannot be accessed. Isn't a file or a directory\n", path);
    }
    free(myPath);
    return;
  }

  //not passed in a complete path form
  strcat(currWorkDir, "/");
  strcat(currWorkDir, myPath);
  if (chdir(currWorkDir))
  {
    perror("cd"); //is this a good way of using perror ?
  }
  else
  {
    chdir(currWorkDir);
  }
  free(myPath);
}

//printenv: built in function, it gets a char** passed to it that represents the envinronment path.
//It prints it out. 
void printenv(char **envp)
{
  char **currEnv = envp;
  while (*currEnv)
  {
    printf("%s \n", *(currEnv++));
  }
}

//freePathlist: helper function, to help clean up memory.
//Frees the pathlist and it's head element
void freePathlist(struct pathelement *headList)
{
  struct pathelement *currElement = headList;
  struct pathelement *tmp = headList;
  free(headList->element);
  while (currElement != NULL)
  {
    tmp = currElement;
    currElement = currElement->next;
    free(tmp);
  }
  free(currElement);
}

//wildCardHandler: helper function that takes a char** of commands and the wildcard position
//It expands the wildcard and puts in memory for the expanded wilcard insie the commands 
//char** that was passed to the function
int wildcardHandler(char **commands, int wildPos)
{
  wordexp_t *cmdExpansion = malloc(sizeof(wordexp_t));
  int index = 0;
  if (wordexp(commands[wildPos], cmdExpansion, 0) == 0)
  {
    while (cmdExpansion->we_wordv[index])
    {
      if(index == 0){
        free(commands[wildPos]);
      }
      commands[index + wildPos] = calloc(strlen(cmdExpansion->we_wordv[index]) + 1, sizeof(char));
      strcpy(commands[index + wildPos], cmdExpansion->we_wordv[index]);
      index++;
    }
    wordfree(cmdExpansion);
  }
  else
  {
    perror("Word expansion for wildcards failed");
  }
  free(cmdExpansion);
  return index + 1;
}

//isDirectory: a helper function, to help determine if a passed path is a Directory or not
//returns 1 if it is a directory and 0 otherwise
int isDirectory(const char *path) {
   struct stat statbuf;
   if (stat(path, &statbuf) != 0)
       return 0;
   return S_ISDIR(statbuf.st_mode);
}

//isDirectory: a helper function, to help determine if a passed path is the path of an executable
// or not returns 1 if it is an executable and 0 otherwise
int isExecutable(const char *path) {
   struct stat statbuf;
   return(stat(path, &statbuf) == 0 && statbuf.st_mode & S_IXUSR);
}