#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define MAX_BUF 1024
#define READ 0
#define WRITE 1
int main(){
        int fdA[2],fdB[2];
        pid_t pid;
        char buf[MAX_BUF];
        int count=0;

        if(pipe(fdA) < 0){
                printf("pipe error\n");
                exit(1);
        }

        if(pipe(fdB) < 0){
                printf("pipe error\n");
                exit(1);
        }

        if((pid=fork())<0){
                printf("fork error\n");
                exit(1);
        }

        printf("\n");
        if(pid>0){ //parent process
                close(fdA[READ]);
                close(fdB[WRITE]);
                while(1){
                        sprintf(buf,"parent %d",count++);
                        write(fdA[WRITE],buf,MAX_BUF);
                        memset(buf,0,sizeof(buf));
                        read(fdB[READ],buf,MAX_BUF);
                        printf("parent got message : %s\n",buf);
                        sleep(1);
                }
        }else{  //child process
                close(fdA[WRITE]);
                close(fdB[READ]);
                count=100000;
                while(1){
                        sprintf(buf,"child %d",count++);
                        write(fdB[WRITE],buf,MAX_BUF);
                        memset(buf,0,sizeof(buf));
                        read(fdA[READ],buf,MAX_BUF);
                        printf("\tchild got message : %s\n",buf);
                        sleep(3);
                }
        }
        exit(0);
}