/*
 * MyShell Project
 * Created by Seungwoo Kim (20181610)
 */
 
#include "myshell.h"

/* Function Prototypes */
/* Phase 1, 2 */
void eval(char *cmdline, int fd[][2], int count, pid_t *pid, sigset_t *prev_one, char *full_cmd);
int builtin_command(char **argv);
int parseline(char *buf, char **argv, char **buf_ptr);
int change_directory(char **argv);

/* Phase 3 */
/* signal handlers */
void sigchld_handler(int sig);
void sigint_handler(int sig);
void sigtstp_handler(int sig);

/* job queue를 관리하는 functions */
void init_job_list();
void add_job(pid_t *pid, char *cmd, int status, int number);
pid_t delete_job(pid_t pid);
void print_jobs();

/* kill, fg, bg 명령어를 위한 functions */
void sus_to_bg(int job_num);
void bg_to_fg(int job_num);
void kill_bg_job(int job_num);

/* main function */
int main(){
  char cmdline[MAXLINE];
  int fd[MAXPIPE][2];
  pid_t pid[MAXPIPE];
  char full_cmd[MAXLINE];
  signal(SIGCHLD, sigchld_handler);
  signal(SIGINT, sigint_handler);
  signal(SIGTSTP, sigtstp_handler);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGTTOU, SIG_IGN);

  sigset_t mask_one, prev_one;
  sigemptyset(&mask_one);
  sigaddset(&mask_one, SIGINT);

  init_job_list();
  shell_pgid = getpid();
  setpgid(shell_pgid, shell_pgid);
  tcsetpgrp(STDIN_FILENO, shell_pgid);
  tcgetattr(STDIN_FILENO, &shell_tmodes);
  do{
    sigprocmask(SIG_BLOCK, &mask_one, &prev_one);
    full_cmd[0] = '\0';
    printf("CSE4110:P4-myshell> ");
    fgets(cmdline, MAXLINE, stdin);
    if (feof(stdin))
      exit(0);
    
    eval(cmdline, fd, 0, pid, &prev_one, full_cmd);
    sigprocmask(SIG_SETMASK, &prev_one, NULL);
  } while(1);
}

/*
 * ---------------------------------------------------------------------
 * Phase 1, 2
 * ---------------------------------------------------------------------
 * eval : cmd을 수행하는 함수
 * builtin_command : 빌트인 명령어를 검출하고 처리하는 함수
 * parseline : 사용자의 명령어를 파싱하는 함수
 * change_directory : cd 명령어 수행을 위한 함수
 * ---------------------------------------------------------------------
 */
void eval(char *cmdline, int fd[][2], int count, pid_t *pid, sigset_t *prev_one, char *full_cmd){
  char *argv[MAXARGS];
  char buf[MAXLINE];
  char new_buf[MAXLINE];
  int bg;
  char *buf_ptr;
  char *pipe_exists;
  int child_status;

  sigset_t mask_all;
  sigfillset(&mask_all);

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

  for (int i = 0 ; argv[i] != NULL ; i++){
    strcat(full_cmd, argv[i]);
    strcat(full_cmd, " ");
  }
  if (pipe_exists)
    strcat(full_cmd, "| ");

  strncpy(new_buf, buf_ptr, MAXLINE);
  if (argv[0] == NULL)
    return;
  if (!builtin_command(argv)){
    // execute
    if ((pid[count] = fork()) == 0){ // Child process
        sigprocmask(SIG_SETMASK, prev_one, NULL);
		    if (count > 0)
          dup2(fd[count-1][READ], STDIN_FILENO);
        if (pipe_exists)
        	dup2(fd[count][WRITE], STDOUT_FILENO);
    	  int execve_status;
        int i = -1;
        char cmd_buf[MAXLINE];
        char orig_cmd[MAXLINE];
        char *bin_usr_bin[2] = {"/bin/", "/usr/bin/"};
        strcpy(orig_cmd, argv[0]);
        do{
        	execve_status = execve(argv[0],argv,environ);
        	if (execve_status < 0) {
          		// /bin, /usr/bin 차례로 붙여서 실행
              i++;
              if (i==2)
                break;
          		cmd_buf[0] = '\0';
          		strcat(cmd_buf, bin_usr_bin[i]);
          		strcat(cmd_buf, orig_cmd);
          		argv[0] = cmd_buf;
        		}
        	else
          		break;
      	} while (1);
      	if (i == 2){
       		printf("%s: Command not found.\n", orig_cmd);
        	exit(0);
      	}
	  }
    else {   // Parent process
      if (pipe_exists) {
          close(fd[count][WRITE]);
          eval(new_buf, fd, ++count, pid, prev_one, full_cmd);
      }
      else{
        int is_stopped = 0;
        int child_num = 0;
        pid_t stopped_pid[MAXPIPE] = {-1,};
        if (!bg){
          int i;
          for(i=0;i<=count;i++){
              waitpid(pid[i], &child_status, WUNTRACED);
              /* 자식이 멈췄을 경우 (ctrl+z) */
              if (WIFSTOPPED(child_status)) {
                sigprocmask(SIG_BLOCK, &mask_all, NULL);
                is_stopped = 1;
                stopped_pid[child_num++] = pid[i];
              }
          }
        }
        else {
          /* Phase 3 */
          sigprocmask(SIG_BLOCK, &mask_all, NULL);
          add_job(pid, full_cmd, RUNNING, count+1);
        }

        if (is_stopped) {
          add_job(stopped_pid, full_cmd, STOPPED, child_num);
        }
      }
    }
  }
  return;
}

int builtin_command(char **argv){
  int i;
  if (!strcmp(argv[0], "exit")) /* quit command */
    exit(0);  
  if (!strcmp(argv[0], "&")){    /* Ignore singleton & */
    return 1;
  }
  if (!strcmp(argv[0], "cd")){ // cd는 예외처리
    change_directory(argv);
    return 1;
  }

  /* Phase 3 */
  if (!strcmp(argv[0], "bg")){
    char argv_copy[MAXLINE];
    argv_copy[0] = '\0';
    strcpy(argv_copy, argv[1]);
    argv_copy[0] = '0';
    int job_num = atoi(argv_copy);
    sus_to_bg(job_num);
    return 1;
  }
  if (!strcmp(argv[0], "fg")){
    char argv_copy[MAXLINE];
    argv_copy[0] = '\0';
    strcpy(argv_copy, argv[1]);
    argv_copy[0] = '0';
    int job_num = atoi(argv_copy);
    bg_to_fg(job_num);
    return 1;
  }
  if (!strcmp(argv[0], "jobs")){
    print_jobs();
    return 1;
  }
  if (!strcmp(argv[0], "kill")){
    char argv_copy[MAXLINE];
    argv_copy[0] = '\0';
    strcpy(argv_copy, argv[1]);
    argv_copy[0] = '0';
    int job_num = atoi(argv_copy);
    kill_bg_job(job_num);
    return 1;
  }
  return 0;                     /* Not a builtin command */
}

int parseline(char *buf, char **argv, char **buf_ptr){
  char *delimiter_ptr;    /* Points to first space delimiter */
  char *pipe_ptr;         /* Points to first pipe or NULL */
  int arg_num;
  int is_bg;
  char *quotes_ptr;

  char *quote_ptr = NULL;
  char argv_with_quote[MAXLINE];
  argv_with_quote[0] = '\0';

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
    // quote 처리
    if (arg_num > 0){
      // quote_ptr를 이용해 마지막 quote의 위치까지 이동
      quote_ptr = buf;
      char *tmp, *search_end;
      if (pipe_ptr)
        search_end = pipe_ptr;
      else
        search_end = &buf[strlen(buf) - 1];
      
      while((tmp = strchr(quote_ptr, '\'')) && tmp < search_end){
        quote_ptr = strchr(quote_ptr, '\'');
        quote_ptr++;
      }
      while((tmp = strchr(quote_ptr, '\"')) && tmp < search_end){
        quote_ptr = strchr(quote_ptr, '\"');
        quote_ptr++;
      }

      // 마지막 quote를 기준으로 빈 칸의 포인터를 반환받음
      if (quote_ptr != buf)
        delimiter_ptr = strchr(quote_ptr, ' ');
    }
    *delimiter_ptr = '\0';

    // 만약 첫번째 인자가 아니고, quote가 존재했다면
    if ((arg_num > 0) && (quote_ptr != buf)){
      char *move;
      //buf = '\"';
      for(move = buf ; move <= delimiter_ptr ; move++){
        if ((*move == '\'') || (*move == '\"') || (*move == '\0')){
          *move = '\0'; // quote를 전부 문자열의 끝으로('\0') 처리
          strcat(argv_with_quote, buf); // argv_with_quote 버퍼에 문자열을 합침
          buf = move + 1;
        }
      }
      argv[arg_num++] = argv_with_quote;
    }
    else{
      argv[arg_num++] = buf;
      buf = delimiter_ptr + 1;
    }

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

int change_directory(char **argv){
  if (argv[1] == NULL)
    return 0;
  int res = chdir(argv[1]);
  if (res)
    fprintf(stderr, "%s: no such file or directory: %s\n", argv[0], argv[1]);
  return 0;
}


/*
 * ---------------------------------------------------------------------
 * Phase 3
 * ---------------------------------------------------------------------
 * [1] Signal handlers
 * sigchld_handler : SIGCHLD 핸들러
 * sigint_handler : SIGINT 핸들러
 * sigtstp_handler : SIGTSTP 핸들러
 * ---------------------------------------------------------------------
 * [2] Job queue management functions
 * init_job_list : Job queue 초기화 함수
 * add_job : Job queue에 Job을 추가하는 함수
 * delete_job : Job queue에서 Job을 제거하는 함수
 * print_jobs : Job queue의 Job을 전부 출력하는 함수
 * ---------------------------------------------------------------------
 * [3] Background, Foreground switching and Kill functions
 * sus_to_bg : 중지된 job을 background로 실행시키는 함수
 * bg_to_fg : Background 실행중이거나 중지된 job을 Foreground에서 실행시키는 함수
 * kill_bg_job : Background 실행중인 job을 kill하는 함수
 * ---------------------------------------------------------------------
 */

/* [1] Signal handlers */
void sigtstp_handler(int sig){
  int olderrno = errno;
  tcsetpgrp(STDIN_FILENO, shell_pgid);
  tcsetattr(STDIN_FILENO, TCSADRAIN, &shell_tmodes);
  errno = olderrno;
}

void sigint_handler(int sig){
  int olderrno = errno;
  sigset_t mask_all, prev_all;
  sigfillset(&mask_all);
  sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
  printf("\n");
  tcsetpgrp(STDIN_FILENO, shell_pgid);
  tcsetattr(STDIN_FILENO, TCSADRAIN, &shell_tmodes);

  sigprocmask(SIG_SETMASK, &prev_all, NULL);
  errno = olderrno;
}

void sigchld_handler(int sig){
  int olderrno = errno;
  sigset_t mask_all, prev_all;
  pid_t pid;

  tcsetpgrp(STDIN_FILENO, shell_pgid);
  tcsetattr(STDIN_FILENO, TCSADRAIN, &shell_tmodes);

  sigfillset(&mask_all);

  if ((pid = waitpid(-1, NULL, WNOHANG)) > 0){
    sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
    delete_job(pid);
    sigprocmask(SIG_SETMASK, &prev_all, NULL);
  }
  errno = olderrno;
}

/* [2] Job queue management functions */
void init_job_list(){
  JQ = (Jobqueue *)malloc(sizeof(Jobqueue));
  JQ->front = JQ->rear = NULL;
  JQ->count = 0;
}

void add_job(pid_t *pid, char *cmd, int status, int number){
  int i;
  Job *newJob = (Job *)malloc(sizeof(Job));

  for (i = 0 ; i < MAXJOBS ; i++)
    newJob->pid[i] = -1;   // -1로 초기화
  
  for (i = 0 ; i < number ; i++)
    newJob->pid[i] = pid[i];

  newJob->pid[number] = 0; // 마지막 pid를 구분하기 위해 다음 원소는 0으로 갱신
  newJob->pgid = newJob->pid[0];
  newJob->number = number;
  newJob->status = status;
  newJob->next = NULL;
  newJob->cmd[0] = '\0';
  tcgetattr(STDIN_FILENO, &newJob->tmodes);
  strcat(newJob->cmd, cmd);

  for (i = 0 ; i < number ; i++)
    setpgid(newJob->pid[i], newJob->pgid);

  if (JQ->count == 0){
    JQ->front = newJob;
    JQ->rear = newJob;
  }
  else {
    JQ->rear->next = newJob;
    JQ->rear = newJob;
  }
  JQ->count++;
}

pid_t delete_job(pid_t pid){
  int i;
  Job *curr;
  Job *prev;
  Job *delJob;
  curr = prev = JQ->front;
  if (JQ->front == NULL) // 큐가 비어있다면
    return -1;
  
  while(curr != NULL) {
    for (i=0 ; curr->pid[i] != 0 ; i++){
        if (curr->pid[i] == pid){
          pid_t del_pid;
          delJob = curr;

          /* 만약 job의 모든 프로세스가 종료되지 않았다면 */
          if (delJob->number > 1){
            delJob->number--;
            del_pid = delJob->pid[i];
            delJob->pid[i] = -1;
            return del_pid;
          }
          if (curr == prev) { // 첫번째 노드라면
            JQ->front = JQ->front->next;
          }
          else if (curr->next == NULL) { // 마지막 노드라면
            prev->next = curr->next;
            JQ->rear = prev;
          }
          else { // 중간 노드라면
            prev->next = curr->next;
          }
          for (int j = 0 ; j < MAXJOBS ; j++){
            if (delJob->pid[j] != -1){
              del_pid = delJob->pid[j];
              break;
            }
          }
          free(delJob);
          JQ->count--;
          return del_pid;
        }
    }
    prev = curr;
    curr = curr->next;
  }
  return -1;
}

void print_jobs(){
  Job *move;
  char *status_str[2] = {"running", "suspended"};
  int i;
  for (move = JQ->front, i = 1; move != NULL ; move = move->next, i++)
      printf("[%d] %s %s\n", i, status_str[move->status], move->cmd);
  return;
}

/* [3] Background, Foreground switching and Kill functions */
void sus_to_bg(int job_num){
  sigset_t mask_all, prev_one;
  sigfillset(&mask_all);
  Job *move;
  int i;
  for (move = JQ->front, i = 1; move != NULL && i != job_num ; move = move->next, i++);

  if (move == NULL){
    printf("No such job\n");
    return;
  }

  for (i=0 ; i < move->number ; i++){
    if (move->pid[i] == 0)
      break;
    else if (move->pid[i] != -1){
      kill(move->pid[i], SIGCONT);
    }
  }
  move->status = RUNNING;
  return;
}

void bg_to_fg(int job_num){
  Job *move;
  int i;
  pid_t pid;

  pid_t wait_pids[MAXJOBS];
  int wait_number;
  for (move = JQ->front, i = 1; move != NULL && i != job_num ; move = move->next, i++);

  if (move == NULL){
    printf("No such job\n");
    return;
  }
  wait_number = move->number;
  int idx = 0;
  tcsetpgrp(STDIN_FILENO, move->pgid);
  // continue
  for (i=0 ; i < move->number ; i++){
    if (move->pid[i] == 0)
      break;
    else if (move->pid[i] != -1){
      wait_pids[idx++] = move->pid[i];
      kill(move->pid[i], SIGCONT);
    }
  }
  move->status = RUNNING;

  int child_status;
  for (i=0;i<wait_number;i++){
    pid = waitpid(wait_pids[i], &child_status, WUNTRACED);
    if (WIFSTOPPED(child_status)){
      //stopped
      move->status = STOPPED;
    }
    else{
      delete_job(pid);
    }
  }
  tcsetpgrp (STDIN_FILENO, shell_pgid);
  tcgetattr(STDIN_FILENO, &move->tmodes);
  tcsetattr (STDIN_FILENO, TCSADRAIN, &shell_tmodes);

  return;
}

void kill_bg_job(int job_num){
  Job *move;
  int i;
  for (move = JQ->front, i = 1; move != NULL && i != job_num ; move = move->next, i++);

  if (move == NULL){
    printf("No such job\n");
    return;
  }

  int pid_number = move->number;
  for (i=0 ; i < pid_number ; i++){
    if (move->pid[i] == 0)
      break;
    else if (move->pid[i] != -1){
      kill(move->pid[i], SIGKILL);
      waitpid(-1, NULL, WUNTRACED);
      delete_job(move->pid[i]);
    }
  }
  return;
}
