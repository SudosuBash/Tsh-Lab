#include <eval.h>
#include <cmd_parser.h>
#include <p_string.h>
#include <unistd.h>
#include <job.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <file.h>

void pipe_eval(char cmdline[MAXLINE]) {
    //管道解析器
    int bg = isbg(cmdline); //是否为background进程

    pstring_array cmds;
    pstring_array cmd_arr[MAXARGS] = {0};
    //处理子程序
    int cmd_len = 0;
    char c = sep_pipes(cmdline,&cmds);
    if(c != 0) {
        printf("E: syntax error near %c.\n",c);
        goto release;
    }
    for(int i=0;i<cmds->arr_length;i++) {
        pstring_array arr;
        c = sep_redirect(cmds->strings[i]->buf,&arr);
        if(c != 0) {
            printf("E: syntax error near %c.\n",c);
            goto release;
        }
        cmd_arr[cmd_len++] = arr;
    }

    int lastfd = STDIN_FILENO;
    int jobid = addjob(cmdline); //获得jobid
    for(int i=0;i<cmds->arr_length;i++) {
        char* argv[MAXLINE] = {0};
        pstring_array n_cmds = cmd_arr[i];
        parseline(n_cmds->strings[0]->buf,argv); //处理程序


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
            
            //替换文件
            for(int j=1;j<n_cmds->arr_length;j++) {
                char* buf = n_cmds->strings[j]->buf;
                int direction = (buf[0] == '>'); //1:stdout 0:stdin
                int fd = file_open(buf);
                buf++;
                if(fd == -1) {
                    fprintf(stderr,"E: Open %s file failed: ",buf);
                    perror("");
                    _exit(0);
                }
                if(direction) {
                    dup2(fd,STDOUT_FILENO);
                } else {
                    dup2(fd,STDIN_FILENO);
                }
                close(fd); //最后别忘记关流，否则会出错
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
                    fprintf(stderr,"E: program %s exec failed: ",argv[0]);
                    perror("");
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
release:
    free_str_arr(&cmds);
    for(int i=0;i<cmd_len;i++) {
        free_str_arr(&cmd_arr[i]);
    }
    return;
}