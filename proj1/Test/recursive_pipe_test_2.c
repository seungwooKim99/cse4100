#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CTRL(x) (#x[0]-'a'+1)
#define MAXARGS 128
#define MAX_BUF 1024
#define MAXLINE 1024
#define READ 0
#define WRITE 1
extern char **environ; /* Defined by libc */
int recursive_eval(char *cmdline, int fd[][2], int count, pid_t *pid);
int parseline(char *buf, char **argv, char **buf_ptr);

int recursive_eval(char *cmdline, int fd[][2], int count, pid_t *pid){
    char *argv[MAXARGS];
    char buf[MAXLINE];
    char new_buf[MAXLINE];
    char *buf_ptr;
    char *pipe_exists;
    int child_status = 0;

    char tmp_buf[MAXLINE];

    char child_output[MAXLINE];

    strcpy(buf, cmdline);
    pipe_exists = strchr(buf, '|');

    if (pipe_exists){
        if(pipe(fd[count]) < 0){
            printf("pipe error\n");
            exit(1);
        }
    }

    // 앞 커맨드 파싱
    parseline(buf, argv, &buf_ptr);
    strncpy(new_buf, buf_ptr, MAXLINE - strlen(buf_ptr));

    if ((pid[count] = fork()) == 0) { // child
        if (count > 0) {
          dup2(fd[count-1][READ], STDIN_FILENO);
          //close(fd[count-1][READ]);
        }
        if (pipe_exists)
          dup2(fd[count][WRITE], STDOUT_FILENO);
        //sleep(1);
        if (execve(argv[0], argv, environ) < 0) {	//ex) /bin/ls ls -al &
            printf("Command not found.\n");
            exit(0);
        }
    }
    else { // parent
        //pid_t wpid = waitpid(pid[count], &child_status, 0);
        close(fd[count][WRITE]);
        pid_t wpid = wait(&child_status);
        //read(fd[READ],tmp_buf,MAXLINE);
        //printf("child got message : %s\n",tmp_buf);
        //dup2(fd[READ], STDIN_FILENO);
        //if (WIFEXITED(child_status)){
          //printf("Child %d terminated with exit status %d\n", wpid, WEXITSTATUS(child_status));

        //}
        //else{
          //printf("Child %d terminate abnormally\n", wpid);
        //}
        if (pipe_exists){
          //close(fd[count][WRITE]);
          recursive_eval(new_buf, fd, ++count, pid);
        }
        else{
          //read(fd[count - 1][READ],tmp_buf,MAXLINE);
          ///printf("child got message : %s\n",tmp_buf);
          printf("Done\n");
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
    //printf("%s\n", buf);
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

    return bg;
}

int main(){
        int fd[MAXLINE][2];
        pid_t pid[MAXLINE];
        char *argv[MAXARGS];
        char buf[MAXLINE];
        char temp[MAXLINE];
        char *buf_ptr;

        //if(pipe(fd) < 0){
        //        printf("pipe error\n");
        //        exit(1);
        //}
        //dup2(fd[READ], STDIN_FILENO);
        //dup2(fd[WRITE], STDOUT_FILENO); // stdout이 fd[WRITE]이 되도록
        //if (pid[0]) {
          // main parent

        //}
        //else {
          strcpy(buf, "/bin/cat test.txt | /usr/bin/grep m | /usr/bin/grep kim\n");
          //strcpy(buf, "/bin/cat test.txt | /bin/ls -al | /bin/ls\n");
          recursive_eval(buf, fd, 0, pid);
          exit(0);
        //}
}