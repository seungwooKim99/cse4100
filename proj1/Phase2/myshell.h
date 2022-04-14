/*
 * MyShell Project
 * Created by Seungwoo Kim (20181610)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <termios.h>

#define MAXARGS 128
#define	MAXLINE 8192
#define MAXPIPE 1024
#define MAXJOBS 1024

/* pipe에 의해 생성되는 fd의 종류 정의 */
#define READ 0
#define WRITE 1

/* Job의 상태 정의 */
#define RUNNING 0
#define STOPPED 1

/* Job 정의 */
typedef struct Job {
  pid_t pid[MAXJOBS];
  pid_t pgid;
  int status;
  int number;
  char cmd[MAXLINE];
  struct Job *next;
  struct termios tmodes;
} Job;

/* Job queue 정의 */
typedef struct _Jobqueue {
  Job *front;
  Job *rear;
  int count;
} Jobqueue;

Jobqueue *JQ; // job queue 포인터
pid_t shell_pgid; // 쉘의 pgid
struct termios shell_tmodes; // 터미널의 attribute

/* External variables */
extern int h_errno;    /* Defined by BIND for DNS errors */ 
extern char **environ; /* Defined by libc */
