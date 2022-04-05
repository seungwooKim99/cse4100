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
int recursive_eval(char *cmdline, int fd[2], int count);
int parseline(char *buf, char **argv, char **buf_ptr);

int recursive_eval(char *cmdline, int fd[2], int count){
    //int fd[2];
    pid_t pid;
    char *argv[MAXARGS];
    char buf[MAXLINE];
    char new_buf[MAXLINE];
    char *buf_ptr;
    char *pipe_exists;

    char child_output[MAXLINE];

    //dup2(fd[READ], STDIN_FILENO);
    //dup2(fd[WRITE], STDOUT_FILENO); // stdout이 fd[WRITE]이 되도록
    

    //if(pipe(fd) < 0){
    //            printf("pipe error\n");
    //            exit(1);
    //    }
    
    strcpy(buf, cmdline);
    pipe_exists = strchr(buf, '|');

    // 앞 커맨드 파싱
    parseline(buf, argv, &buf_ptr);
    strncpy(new_buf, buf_ptr, MAXLINE - strlen(buf_ptr));

    // 뒤에도 파이프가 있는가?
    if (pipe_exists){
      if ((pid = fork()) == 0) { // child
        recursive_eval(new_buf, fd, ++count);
      }
      else { // parent
          //close(fd[READ]);
          //printf("PID :%d / AGRV :%s\n",pid,argv[0]);
          printf("count : %d\n", count);
          //dup2(fd[READ], STDIN_FILENO);
          dup2(fd[WRITE], STDOUT_FILENO); // stdout이 fd[WRITE]이 되도록
          if (execve(argv[0], argv, environ) < 0) {	//ex) /bin/ls ls -al &
              printf("Command not found.\n");
              exit(0);
        }
      }
    }
    else {
      // 더이상 pipe가 없다면 바로 execve 실행
        sleep(1);
        //close(fd[WRITE]);
        printf("no more pipe count: %d / PID :%d / AGRV :%s, %s, %s, %s, %s\n",count, pid,argv[0], argv[1], argv[2], argv[3], argv[4]);
        dup2(fd[READ], STDIN_FILENO);
        //dup2(fd[WRITE], STDOUT_FILENO); // stdout이 fd[WRITE]이 되도록
        //read(fd[READ],child_output,MAXLINE);
        //int i = 0;
        //while (argv[i] != NULL)
        //  i++;
        //argv[i] = STDOUT_FILENO;
        //argv[++i] = NULL;
        //printf("argv: %s, %s, %s, %s, %s, %s\n", argv[0], argv[1], argv[2], argv[3], argv[4], argv[5]);
        if (execve(argv[0], argv, environ) < 0) {	//ex) /bin/ls ls -al &
          printf("Command not found.\n");
          exit(0);
        }
    }
    return 0;
}

int parseline(char *buf, char **argv, char **buf_ptr) 
{
    char *delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */
    int bg;              /* Background job? */
    char *pipe_ptr;

    // Pipe가 있나?
    pipe_ptr = strchr(buf, '|');
    printf("%s\n", buf);
    if (pipe_ptr)
      *pipe_ptr = ' ';
    else
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
      if (pipe_ptr && ((pipe_ptr <= delim) || (pipe_ptr <= buf)))
        break;
    }
    *buf_ptr = buf;
    argv[argc] = NULL;
    
    if (argc == 0)  /* Ignore blank line */
	    return 1;

    /* Should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0)
	    argv[--argc] = NULL;
    //printf("argv: %s, %s, %s, %s, %s, %s\n", argv[0], argv[1], argv[2], argv[3], argv[4], argv[5]);

    return bg;
}

int main(){
        int fd[2];
        pid_t pid;
        char *argv[MAXARGS];
        char buf[MAXLINE];
        char temp[MAXLINE];
        char *buf_ptr;

        if(pipe(fd) < 0){
                printf("pipe error\n");
                exit(1);
        }
        //if((pid=fork())<0){
        //        printf("fork error\n");
        //        exit(1);
        //}


        if (pid) {
          // main parent

        }
        else {
          // main child (recursive start)
          //strcpy(buf, "/bin/ls -al | /usr/bin/grep 0 | /usr/bin/grep test\n");
          strcpy(buf, "/bin/cat test.txt | /usr/bin/grep m | /usr/bin/grep kim\n");
          recursive_eval(buf, fd, 0);
          exit(0);
        }
        /*
                printf("main\n");
        strcpy(buf, "aa bb cc | dd ee\n");
        parseline(buf, argv, &buf_ptr);

        printf("argv: %s, %s, %s, %s, %s, %s\n", argv[0], argv[1], argv[2], argv[3], argv[4], argv[5]);
        printf("buf :%s\n", buf);
        printf("buf_ptr :%s\n", buf_ptr);

        strncpy(temp, buf_ptr, MAXLINE);
        strncpy(buf, temp, MAXLINE);
        parseline(buf, argv, &buf_ptr);

        printf("argv: %s, %s, %s, %s, %s, %s\n", argv[0], argv[1], argv[2], argv[3], argv[4], argv[5]);
        printf("buf :%s\n", buf);
        printf("buf_ptr :%s\n", buf_ptr);
        */


        /*
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
        exit(0);
        */
}