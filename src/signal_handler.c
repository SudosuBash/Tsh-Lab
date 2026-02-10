#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <eval.h>
#include <job.h>
#include <errno.h>
#include <stdio.h>

typedef void handler_t(int);

handler_t *Signal(int signum, handler_t *handler) 
{
    struct sigaction action, old_action;

    action.sa_handler = handler;  
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
	    unix_error("Signal error");
    return (old_action.sa_handler);
}

void sigchld_handler(int _) {
    int stat;
    pid_t pid;
    while((pid = waitpid(-1,&stat,WNOHANG | WUNTRACED)) > 0) {
        if(pid < 0 && errno != ECHILD) unix_error("E: waitpid error.");
        if(jpstatus(pid)) {
            pid_t p = getpgrp();
            tcsetpgrp(STDIN_FILENO,p);//交还控制权给终端
        }
        if(WIFSTOPPED(pid)) {
            printf("PID %d Process Stopped.\n",pid);
            update_pid_status(pid,ST);
        } else if(WIFEXITED(pid)) {
            printf("PID %d Process Exited.\n",pid);
            update_pid_status(pid,DONE);
        } else if(WIFSIGNALED(pid)) {
            printf("PID %d Process Signaled.\n",pid);
            update_pid_status(pid,DONE);
        }
    }
}