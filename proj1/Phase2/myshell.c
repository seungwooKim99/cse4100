#include "myshell.h"
#define MAXARGS 128

/*Function Prototypes*/
void eval(char *cmdline);
int builtin_command(char **argv);
int parseline(char *buf, char **argv);
void unix_error(char *msg);
int change_directory(char **argv);

int main(){
  char cmdline[MAXLINE];
  do{
    printf("CSE4110:P4-myshell> ");
    fgets(cmdline, MAXLINE, stdin);
    if (feof(stdin))
      exit(0);
    
    eval(cmdline);
  } while(1);
}

void eval(char *cmdline){
  char *argv[MAXARGS];
  char buf[MAXLINE];
  int bg;
  pid_t pid;

  strcpy(buf, cmdline);
  bg = parseline(buf, argv);
  if (argv[0] == NULL)
    return;
  if (!builtin_command(argv)){
    // execute
    if ((pid = fork()) == 0){
      if (execve(argv[0], argv, environ) < 0){
        printf("%s: Command not found.\n", argv[0]);
        exit(0);
      }
    }
    if (!bg){
      int status;
      if (waitpid(pid, &status, 0) <0){
        unix_error("waitfg: waitpid error");
      }
    }
    else{
      printf("%d %s\n", pid, cmdline);
    }
  }
  return;
}

int builtin_command(char **argv) 
{
  if (!strcmp(argv[0], "exit")) /* quit command */
	  exit(0);  
  if (!strcmp(argv[0], "&")){    /* Ignore singleton & */
    return 1;
  }
  if (!strcmp(argv[0], "cd")){ // cd는 예외처리
    change_directory(argv);
    return 1;
  }
  if (!strcmp(argv[0], "ls")){
    argv[0] = "/bin/ls";
  }
  else if (!strcmp(argv[0], "mkdir")){
    argv[0] = "/bin/mkdir";
  }
  else if (!strcmp(argv[0], "rmdir")){
    argv[0] = "/bin/rmdir";
  }
  else if (!strcmp(argv[0], "touch")){ // linux server에서 /usr 빼기!
    argv[0] = "/usr/bin/touch";
  }
  else if (!strcmp(argv[0], "cat")){
    argv[0] = "/bin/cat";
  }
  else if (!strcmp(argv[0], "echo")){
    argv[0] = "/bin/echo";
  }
  return 0;                     /* Not a builtin command */
}

int parseline(char *buf, char **argv){
  char *delimiter_ptr;
  int arg_num;
  int is_bg;

  buf[strlen(buf)-1] = ' '; // '\n' to ' '
  while (*buf && (*buf == ' ')) // Ignore leading spaces
    buf++;
  
  // make argument array
  arg_num = 0;
  while ((delimiter_ptr = strchr(buf, ' '))){
    argv[arg_num++] = buf;
    *delimiter_ptr = '\0';
    buf = delimiter_ptr + 1;
    while (*buf && (*buf == ' '))
      buf++;
  }
  argv[arg_num] = NULL;
  
  if (arg_num == 0) // no args (blank line)
    return 1;
  
  if ((is_bg = (*argv[arg_num-1] == '&'))) // bg라면 마지막 argv는 NULL을 가리키도록
    argv[--arg_num] = NULL;
  return is_bg;
}

void unix_error(char *msg){
  fprintf(stderr, "%s: %s\n", msg, strerror(h_errno));
  exit(0);
}

int change_directory(char **argv){
  if (argv[1] == NULL)
    return 0;
  int res = chdir(argv[1]);
  if (res)
    fprintf(stderr, "%s: no such file or directory: %s\n", argv[0], argv[1]);
  return 0;
}