#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <eval.h>
#include <config.h>
#include <errno.h>
#include <signal_handler.h>
#include <job.h>
#include <unistd.h>
const char* prompt = "tsh";
const char* end = "> ";

void unix_error(char *msg)
{
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

int rd_command(char cmdline[MAXLINE]) { //读取命令
    size_t strl = 0;
    do {
    rd_cmd_loop_start:
        if(strl > 0 && cmdline[strl-1] == '|') 
            printf("pipe");
            
        printf("%s",end);
        char buf[MAXLINE] = {0};
        if ((fgets(buf, MAXLINE, stdin) == NULL) && ferror(stdin))
            unix_error("fgets error");
        
        fflush(stdout);
        size_t len = strlen(buf) - 1; //去掉 '\n'
        if(buf[0] == 0 && strl == 0) break;
        while((buf[len] == ' ' || buf[len] == '\n') && len>=0) 
            len--;

        int cmd_nend = (buf[len] == '\\');
        if(cmd_nend) len--; // '\'代表没结束呢

        memcpy(cmdline+strl,buf,len+1);
        strl += len+1;
        if(cmd_nend) {
            printf("\'\\\'");
            goto rd_cmd_loop_start;
        }
    } while(cmdline[strl-1] == '|');
    return strl != 0;
}

int main() {
    setbuf(stdout,NULL);
    char cmdline[MAXLINE] = {0};
    tcsetpgrp(STDIN_FILENO,getpgrp());//交还控制权给终端
    signal(SIGCHLD,sigchld_handler);
    signal(SIGINT,sigint_handler);
    signal(SIGTSTP,sigtstp_handler);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    while(1) {
        memset(cmdline,0,sizeof(cmdline));
        printf("%s",prompt);

        //管道支持
        if(rd_command(cmdline) == 0) 
            continue; //没有多余的

        if (feof(stdin)) { /* End of file (ctrl-d) */
            fflush(stdout);
            exit(0);
        }
        pipe_eval(cmdline);
        recyclejob();
    }
}