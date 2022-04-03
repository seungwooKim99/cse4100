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
        int fd[2];
        pid_t pid[2];
        char *argv[MAXARGS];
        char buf[MAXLINE];

        if(pipe(fd) < 0){
                printf("pipe error\n");
                exit(1);
        }
        if((pid[0]=fork())<0){
                printf("fork error\n");
                exit(1);
        }

        printf("\n");
        if (!pid[0]) {
          if (!(pid[1] = fork())) {
            // child를 fork한 child (pipe 받을 명령어 수행)
            close(fd[WRITE]);
            read(fd[READ],buf,MAX_BUF);
            printf("child2 got message : %s\n",buf);
            exit(0);
          }
          else {
            // child (메세지를 보낼 프로세스)
            close(fd[READ]);
            strcpy(buf, "/bin/ls -al\n");
            parseline(buf, argv);
            dup2(fd[WRITE], STDOUT_FILENO); // stdout이 fd[WRITE]이 되도록
            
            //printf("This is execve result from child1\n");
            printf("%s, %s", argv[0], argv[1]);
            if (execve(argv[0], argv, environ) < 0) {	//ex) /bin/ls ls -al &
              printf("Command not found.\n");
              exit(0);
            }
          }
        }
        else {
          //parent
        }
        /*
        if(pid[0]>0){ //parent process
                close(fd[READ]);
                dup2(fd[WRITE], STDOUT_FILENO);
                printf("This is execve result\n");
                printf("fd[WRITE]: %d\n", fd[WRITE]);
        }else{  //child process
                close(fd[WRITE]);
                read(fd[READ],buf,MAX_BUF);
                printf("child got message : %s\n",buf);
                printf("fd[READ]: %d\n", fd[READ]);
        }
        */
        exit(0);
}