#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define MAXARGS 128
#define MAX_BUF 1024
#define MAXLINE 1024
#define READ 0
#define WRITE 1
extern char **environ; /* Defined by libc */

int parseline(char *buf, char **argv) 
{
    char *delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */
    int bg;              /* Background job? */

    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
	    buf++;

    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
      argv[argc++] = buf;
      *delim = '\0';
      buf = delim + 1;
      while (*buf && (*buf == ' ')) /* Ignore spaces */
        buf++;
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* Ignore blank line */
	    return 1;

    /* Should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0)
	    argv[--argc] = NULL;

    return bg;
}

int main(){
        int fd[MAXLINE][2];
        pid_t pid[2];
        char *argv[MAXARGS];
        char buf[MAXLINE];
        int count = 0;

        if(pipe(fd[count]) < 0){
                printf("pipe error\n");
                exit(1);
        }

        printf("go\n");
        if (!(pid[count] = fork())) {
          // child (pipe 받을 명령어 수행)
          close(fd[count][READ]);
          //strcpy(buf, "/bin/ls -al\n");
          //parseline(buf, argv);
          dup2(fd[count][WRITE], STDOUT_FILENO); // stdout이 fd[WRITE]이 되도록
          
          printf("This is execve result from child1\n");
          //printf("%s, %s", argv[0], argv[1]);
          exit(0);
        }
        else {
          // parent (메세지를 보낼 프로세스)
          close(fd[count][WRITE]);
          //read(fd[count][READ],buf,MAX_BUF);
          //printf("parent got message from child1 : %s\n",buf);

          count++;
          //dup2(fd[count][READ], STDOUT_FILENO);
          //read(fd[count-1][READ],buf,MAX_BUF);
          //printf("parent got message from child1 : %s\n",buf);

          if(pipe(fd[count]) < 0){
              printf("pipe error\n");
              exit(1);
          }

          if (!(pid[count] = fork())) {
            // child (pipe 받을 명령어 수행)
            //close(fd[count][READ]);
            //strcpy(buf, "/bin/ls -al\n");
            //parseline(buf, argv);
            read(fd[count-1][READ],buf,MAX_BUF);
            dup2(fd[count][WRITE], STDOUT_FILENO); // stdout이 fd[WRITE]이 되도록
            printf("%s\n",buf);
            
            printf("This is execve result from child2\n");
            //printf("%s, %s", argv[0], argv[1]);
            exit(0);
          }
          else{
            //parent
            read(fd[count][READ], buf, MAX_BUF);
            printf("Final : %s", buf);
          }
        }
        exit(0);
}