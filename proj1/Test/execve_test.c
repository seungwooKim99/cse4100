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
        if(pipe(fd) < 0){
                printf("pipe error\n");
                exit(1);
        }
        if((pid[0]=fork())<0){
                printf("fork error\n");
                exit(1);
        }

        printf("\n");
        char *argv[MAXARGS];
        char buf[MAXLINE];
        if (!pid[0]) {
          printf("child pid : %d\n", pid[0]);
        }
        else {
          //parent
          strcpy(buf, "/bin/ls -al\n");
          parseline(buf, argv);
          printf("%s, %s", argv[0], argv[1]);
          if (execve(argv[0], argv, environ) < 0) {	//ex) /bin/ls ls -al &
            printf("Command not found.\n");
            exit(0);
          }
        }
        exit(0);
}