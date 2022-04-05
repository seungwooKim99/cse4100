#include "myshell.h"
#define MAXARGS 128

/*Function Prototypes*/
void eval(char *cmdline, int fd[][2], int count, pid_t *pid);
int builtin_command(char **argv);
int parseline(char *buf, char **argv, char **buf_ptr);
void unix_error(char *msg);
int change_directory(char **argv);

int main(){
  char cmdline[MAXLINE];
  int fd[MAXPIPE][2];
  pid_t pid[MAXPIPE];
  do{
    printf("CSE4110:P4-myshell> ");
    fgets(cmdline, MAXLINE, stdin);
    if (feof(stdin))
      exit(0);
    
    eval(cmdline, fd, 0, pid);
  } while(1);
}

void eval(char *cmdline, int fd[][2], int count, pid_t *pid){
  char *argv[MAXARGS];
  char buf[MAXLINE];
  char new_buf[MAXLINE];
  int bg;
  char *buf_ptr;
  char *pipe_exists;
  int child_status;

  strcpy(buf, cmdline);
  pipe_exists = strchr(buf, '|'); // 파이프가 존재하나?

  // 파이프가 존재하면 pipe 생성
  if (pipe_exists){
    if (pipe(fd[count]) < 0){
      printf("pipe error!\n");
      exit(1);
    }
  }

  // 가장 먼저 등장하는 커맨드 파싱
  bg = parseline(buf, argv, &buf_ptr);
  strncpy(new_buf, buf_ptr, MAXLINE);

  if (argv[0] == NULL)
    return;
  if (!builtin_command(argv)){
    // execute
    if ((pid[count] = fork()) == 0){ // Child process
      if (count > 0)
        dup2(fd[count-1][READ], STDIN_FILENO);
      if (pipe_exists)
        dup2(fd[count][WRITE], STDOUT_FILENO);
      if (execve(argv[0], argv, environ) < 0){
        printf("%s: Command not found.\n", argv[0]);
        exit(0);
      }
    }
    else {   // Parent process
      if (!bg){
        if (waitpid(pid[count], &child_status, 0) < 0)
          unix_error("waitfg: waitpid error");
        if (pipe_exists) {
          close(fd[count][WRITE]);
          eval(new_buf, fd, ++count, pid);
        }
      }
      else {
        printf("%d %s\n", pid[count], cmdline);
      }
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
    argv[0] = "/bin/touch";
  }
  else if (!strcmp(argv[0], "cat")){
    argv[0] = "/bin/cat";
  }
  else if (!strcmp(argv[0], "echo")){
    argv[0] = "/bin/echo";
  }
  return 0;                     /* Not a builtin command */
}

int parseline(char *buf, char **argv, char **buf_ptr){
  char *delimiter_ptr;    /* Points to first space delimiter */
  char *pipe_ptr;         /* Points to first pipe or NULL */
  int arg_num;
  int is_bg;

  /* Pipe가 있는지 확인 */
  pipe_ptr = strchr(buf, '|');
  if (pipe_ptr)
    *pipe_ptr = ' ';  // Pipe를 ' ' 으로 바꿈
  else
    buf[strlen(buf)-1] = ' '; // '\n'을 ' ' 으로 바꿈
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
    if (pipe_ptr && ((pipe_ptr <= delimiter_ptr) || (pipe_ptr <= buf)))
      break;
  }
  *buf_ptr = buf;
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