#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include<signal.h>
#include <stdbool.h>
#define MAX_ARGS 10 //the maximum number of arguments for a single command
#define MAX_LINE 100 //the maximum command length

void executeCommand(char* args[], int cmdNum){
    int childId = fork();

    if (childId  < 0) {
          printf("ERROR: forking child process failed\n");
          exit(1);
     }

    if (childId  == 0) { //child process
          if (execvp(args[0],args) < 0) {   // execute command and return error for invalid command
               printf("ERROR: wrong command!!!\n");
               exit(1);
          }
     }
     else { //parent process
      if(cmdNum == 0){  // not a built in command, not a background process
          waitpid(childId, NULL, 0);
       }
     }
}

int checked(char line[]){
    for(int j = 0; j < strlen(line) - 1; j++){
        if(line[j] == '$'){
                return 6;
            }
    }
    return 7;
}

char *remove_quotes(char *str){
    int i = 0, j = 0;
	char *temp = malloc(sizeof(str)*sizeof(char));
	while(str[i++]) {
		if (str[i - 1] != '"')
			temp[j++] = str[i-1];
	}
	temp[j] = '\0';
	return temp;
}

void removeQuotes(char* args[] , char line[]){
    int i = 0;
    args[i] = strtok(line,"\"");
    while(args[i] != NULL){
        i++;
        args[i] = strtok(NULL,"\"");
    }
}

void writeToLogFile(){
    FILE *fptr;
    fptr = fopen("log.txt","a");
    wait(-1);
    fprintf(fptr, "Child process was terminated\n");
    fclose(fptr);
}

int readInput(char line[]){
    fgets(line, MAX_LINE, stdin); // read a line
    // removing the end line from the end of the command
    int i = 0;
    int len = 0;
    while(line[i] != '\n'){i++;}
    len = i;
    line[i] = '\0';
    i = 0;
   if(strcmp(line, "exit") == 0) { // to exit the shell
         exit(0);
    }
   if(line[strlen(line)-1] == '&' ) {  // if it is a background process
        line[strlen(line)-1] = '\0';
        return 1;
   }
   else if(line[0] == 'c' && line[1] == 'd'){
        if(len == 2){return 22;}
        return 2;
   }
   else if(line[0] == 'e' && line[1] == 'c' && line[2] == 'h' && line[3] == 'o'){
        return 3;
   }
   else if(line[0] == 'e' && line[1] == 'x' && line[2] == 'p' && line[3] == 'o' && line[4] == 'r' && line[5] == 't'){
        return 4;
   }
   else {
        return 0;
   }
}

void evaluatExpression(char command[], char *args[], int cmdNum){
    int i = 0;
    args[i] = strtok(command," ");
    while(args[i] != NULL){
         i++;
         args[i] = strtok(NULL,"$");
    }
    if(cmdNum == 2 || cmdNum == 22){ // cd command
    char *arg[10];
    removeQuotes(arg,args[1]);
    const char *var_name = arg[0];
    char *value = getenv(var_name);
    args[1] = value; // replace the expression with its value
    }
    else if (cmdNum == 3) { // echo command
    char *arg[10];
    removeQuotes(arg, args[2]);
    const char *var_name = arg[0];
    char *value = getenv(var_name);
    args[2] = value;
    }
    else if(cmdNum == 0){ // another command
        char *arg[10];
        removeQuotes(arg,args[1]);
        const char *var_name = arg[0];
        char *value = getenv(var_name);
        args[1] = value;
        args[1] = strtok(args[1]," ");
        int i = 1;
        while(args[i] != NULL){
            i++;
            args[i] = strtok(NULL," ");
        }
    }
}

int parse_line(char* args[] , char line[], int cmdNum){  //function to parse commands and to deal with arguments
  int i = 0;
  if(cmdNum == 2 || cmdNum == 22){ // cd
    args[i] = strtok(line, " ");
    if(args[i] == NULL){
        return 1;
    }
    while(args[i] != NULL){
         i++;
         args[i] = strtok(NULL, " ");
    }
    return 1;
  }
  if(cmdNum == 3){ // echo
    removeQuotes(args, line);
    return 1;
  }
  else if(cmdNum == 4){ // export
    args[i] = strtok(line, " ");
    if(args[i] == NULL){
        return 1;
    }
    while(args[i] != NULL){
         i++;
         args[i] = strtok(NULL, "=");
    }
  }
  else{
    args[i] = strtok(line, " ");
    if(args[i] == NULL){
        return 1;
    }
    while(args[i] != NULL){
         i++;
         args[i] = strtok(NULL, " ");
    }
    return 1;
  }
}

void executeBuiltInShell(char* command[], int cmdNum){
      if (cmdNum == 2 || cmdNum == 22){ // cd
        char cwd[MAX_LINE];
        if (cmdNum == 22){ // cd
            getcwd(cwd, 100);
            chdir(cwd);
            printf("%s\n",cwd);
        }
        else if (chdir(command[1]) == 0) { //success
            printf("%s\n", getcwd(cwd, 100));
        }
        else if(strcmp(command[1], "~") == 0){
            chdir("/home/salma");
            printf("%s\n", getcwd(cwd, sizeof(cwd)));
        }
        else{
            perror("No such directory");
        }
      } // end of cd
      if(cmdNum == 3){ // echo
            for(int i = 1; i < strlen(command)-2 && command[i] != '\0'; i++){
                if(command[i][0] == '"'){
                    command[i] = remove_quotes(command[i]);
                }
                printf("%s", command[i]);
            }
            printf("\n");
    } // end of echo
    else if(cmdNum == 4){ // export
            for(int i = 1; i < strlen(command)-2 && command[i] != '\0'; i++){
                if(command[i][0] == '"'){
                    command[i] = remove_quotes(command[i]);
                }
            }
            const char *var_name = command[1];
            const char *new_value = command[2];
            setenv(var_name, new_value, 1);
            printf("%s \n", getenv(var_name));
    } // end of export
}

void setupEnvironment(){
    char s[100];
    getcwd(s, 100);
    chdir(s);
    printf("%s\n",s);
}

void sigChild(int signal){
    int status;
    int pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {}
    writeToLogFile();
}

void shell(){
    char* args[MAX_ARGS];
    char line[MAX_LINE];
    int cmdNum;
    int check;
    signal(SIGCHLD, sigChild); //so when a child dies, the parent process receives SIGCHLD signal
    while(1){
        printf("Enter your command >>> ");
        cmdNum = readInput(line);
        check = checked(line);
        if(check == 6){  //there is $ in the expression
            evaluatExpression(line, args, cmdNum);
        }
        else{
             parse_line(args, line, cmdNum);
        }
        if(cmdNum == 2||cmdNum == 22 || cmdNum == 3 || cmdNum == 4){ //cd    echo    export
            executeBuiltInShell(args, cmdNum);
        }
        else{
            executeCommand(args,cmdNum);
        }

    }
}

int main()
{
   FILE *fptr;
   fptr = fopen("log.txt", "w");
   //fprintf(fptr, "%s", "");
   setupEnvironment();
   shell();
}
