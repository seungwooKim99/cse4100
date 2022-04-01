# CSE4100 System Programming Assignment
# Project I : MyShell
## Phase I : Building and Testing Your Shell
### Task Specifications
Your first task is to write a simple shell and starting processes is the main function of linux shells. So, writing a shell means that you need to know exactly what is going on with processes and how they start.
Your shell should be able to execute the basic internal shell commands such as,
- cd, cd..: to navigate the directories in your shell
- ls: listing the directory contents
- mkdir, rmdir: create and remove directory using your shell
- touch, cat, echo: creating, reading and printing the contents of a file
- exit: exit all the child processes and shell quits
A command is always executed by the child process created via forking by the parent process except cd.
## Phase II: Redirection and Piping in Your Shell
### Task Specifications
In this phase, you will be extending the functionality of the simple shell example that you programmed in project phase I. Start by creating a new process for each command in the pipeline and making the parent wait for the last command. This will allow running simple commands such as “ls -al | grep filename”. The key idea is; passing output of one process as input to another. Note that, you can have multiple chains of pipes as your command line argument.
## Phase III: Run Processes in Background in Your Shell
### Task Specifications:
It is the last phase of your MyShell project, where you enable your shell to run processes in background. Linux shells support the notion of job control, which allows users to move jobs back and forth between background and foreground, and to change the process state (running, stopped, or terminated) of the processes in a job.
Your shell must start a command in the background if an ‘&’ is given in the command line arguments. Besides, your shell must also provide various built-in commands that support job control.

For example:
- jobs: List the running and stopped background jobs.
- bg job: Change a stopped background job to a running background job.
- fg job: Change a stopped or running background job to a running in the foreground.
- kill job: Terminate a job.

Note that, one should not be required to separate the ‘&’ from the command by a space. For example, the commands ‘sort foo.txt &’, and ‘sort foo.txt&’ and ‘sort foo.txt &’ (blanks after the ampersand) are all valid.