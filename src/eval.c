#include <eval.h>
#include <cmd_parser.h>
#include <p_string.h>
#include <unistd.h>
#include <job.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

void pipe_eval(char cmdline[MAXLINE]) {
    //管道解析器
    int bg = isbg(cmdline); //是否为background进程
    pstring_array cmds;
    sep_pipes(cmdline,&cmds);

    int lastfd = STDIN_FILENO;

    int jobid = addjob(cmdline); //获得jobid
    for(int i=0;i<cmds->arr_length;i++) {
        char* argv[MAXLINE] = {0};
        parseline(cmds->strings[i]->buf,argv);

        //有些怪胎指令不能fork，否则出大问题
        int ret = builtin_cmd_not_fork(argv,i);
        if(ret == -1) {
            printf("tsh: W: %s cmd in the pipe. ignoring...\n",argv[0]);
            continue;//不执行
        } else if(ret == 1) {
            return; //执行完毕,返回,没必要执行
            //那addjob的jobid呢?让它自然recycle去
        }

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
            signal(SIGCHLD, SIG_DFL);
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
            if(builtin_cmd(argv)) 
                _exit(0);
            //关闭管道
            int res = execvp(argv[0],argv);
            if(res < 0) {
                if(errno == ENOENT) {
                    printf("E: command %s not found.\n",argv[0]);
                    _exit(0);
                } else {
                    unix_error("execvp error.\n");
                    _exit(0);
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