#include <eval.h>
#include <cmd_parser.h>
#include <p_string.h>
#include <unistd.h>
#include <job.h>
#include <errno.h>
#include <stdio.h>
void pipe_eval(char cmdline[MAXLINE]) {
    //管道解析器
    pstring_array cmds;
    sep_pipes(cmdline,&cmds);

    int bg = isbg(cmdline); //是否为background进程
    int lastfd = STDIN_FILENO;

    int jobid = addjob(cmdline); //获得jobid
    for(int i=0;i<cmds->arr_length;i++) {
        char* argv[MAXLINE] = {0};
        parseline(cmds->strings[i]->buf,argv);
        
        int pfd[2] = {0};
        if(i!=cmds->arr_length-1) {
            pipe(pfd);
        }
        pid_t p = fork();
        if(p < 0) {
            unix_error("fork error.");
        }
        if(p == 0) {
            signal(SIGINT, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            usleep(10);

            
            if(i!=0){
                dup2(lastfd,STDIN_FILENO);
                close(lastfd);
            }

            if(i!=cmds->arr_length-1) {
                dup2(pfd[1],STDOUT_FILENO);
                close(pfd[1]);
            }
            //关闭管道
            int res = execvp(argv[0],argv);
            if(res < 0) {
                if(errno == ENOENT) {
                    printf("E: command %s not found.\n",argv[0]);
                    _exit(0);
                } else {
                    unix_error("execvp error.\n");
                }
            }
        } else {
            addpid(jobid,p,bg ? BG : FG); //任务加pid
            if(i!=cmds->arr_length-1) close(pfd[1]);
            if(i!=0)close(lastfd);
            lastfd = pfd[0];
        }
    }
    if(!bg) {
        waitfg(jobid);
    } else {
        printf("[%d]: + background %s\n",jobid,cmdline);
    }
}