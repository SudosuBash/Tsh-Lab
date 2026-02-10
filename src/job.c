#include <job.h>

#include <config.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
static job_t jobs[MAXJOBS] = {0};

static int fg_jid = -1; //前台jid
int addjob(char cmd[MAXLINE]) {
    for(int i=0;i<MAXJOBS;i++) {
        if(jobs[i].pgid == 0) {
            strcpy(jobs[i].cmd,cmd);
            return i;
        }
    }    
    return -1;
}

pid_t jid2pgid(int jid) {
    return jobs[jid].pgid;
}

void addpid(int jid,int pid,pid_stat_t stat) {
    if(jobs[jid].pgid == 0) {
        jobs[jid].pgid = pid;
    }
    if(stat == FG) { 
        //新增状态的，stat改为FG
        fg_jid = jid; 
    }
    setpgid(pid,jobs[jid].pgid); //设置进程组
#if DEBUG
    printf("Added job %d pgid %d\n",jid,getpgid(pid));
#endif
    int cnt = jobs[jid].job_count;
    jobs[jid].jobs[cnt].pid = pid;
    jobs[jid].jobs[cnt].stat = stat;
    jobs[jid].jobs[cnt].jid = jid;
    jobs[jid].job_count++;
    jobs[jid].stat[stat-1]++;
}


pid_stat_t jstatus(int jid) {
    job_t j = jobs[jid];
    if(j.pgid == 0) return UNDEF;
    if(JOB_IS_BACKGROUND(&j) != 0) 
        return BG;
    if(JOB_IS_FOREGROUND(&j) != 0)
        return FG;
    if(JOB_DONE_COUNT(&j) == JOB_COUNT(&j)) 
        return DONE;
    if(JOB_STOPPED_COUNT(&j) + JOB_DONE_COUNT(&j) == JOB_COUNT(&j))
    //有些DONE了，有些还STOP呢
        return ST;

    return UNDEF;
}

static char* stat_to_str(pid_stat_t stat) {
    switch (stat)
    {
    case BG: return "background";
    case FG: return "foreground";
    case ST: return "stopped";
    case DONE: return "done";
    default: return "undefined";
    }
}

void recyclejob() {
    for(int i=0;i<MAXJOBS;i++) {
        if(jstatus(i) == DONE) {
            memset(&jobs[i], 0 ,sizeof(job_t));
        } //全部清理掉
    }
}

void waitfg(int jid) {
    tcsetpgrp(STDIN_FILENO, jobs[jid].pgid);
    // //把控制权交给作业
    while(JOB_IS_FOREGROUND(&jobs[jid])) {
        //这儿确实存在竞争条件
        pause();
    }

    // sigset_t mask, oldmask;
    
    // // 阻塞 SIGCHLD 信号
    // sigemptyset(&mask);
    // sigaddset(&mask, SIGCHLD);
    // sigprocmask(SIG_BLOCK, &mask, &oldmask);
    
    // // 将控制权交给前台作业
    // tcsetpgrp(STDIN_FILENO, jobs[jid].pgid);
    
    // // 等待作业状态改变
    // while (JOB_IS_FOREGROUND(&jobs[jid])) {
    //     // 原子性地解除阻塞并等待信号
    //     sigsuspend(&oldmask);
    // }
    
    // // 恢复信号屏蔽
    // sigprocmask(SIG_SETMASK, &oldmask, NULL);
}

int pid2jid(pid_t pid) {
    for(int i=0;i<MAXJOBS;i++) {
        for(int j=0;j<jobs[i].job_count;j++) {
            if(jobs[i].jobs[j].pid == pid) 
                return i;
        }
    }
    return -1;
}
int pgid2jid(pid_t pgid) {
    for(int i=0;i<MAXJOBS;i++) {
        if(jobs[i].pgid == pgid) return i;
    }
    return -1;
}

void pgid_stat_fg(int pgid) {
    int jid = pgid2jid(pgid);
    for(int i=0;i<jobs[jid].job_count;i++) {
        if(jobs[jid].jobs[i].stat != DONE) {
            jobs[jid].stat[jobs[jid].jobs[i].stat-1] -= 1;
            jobs[jid].jobs[i].stat = FG;
            jobs[jid].stat[FG-1] += 1;
        }
    }
    if(jid == fg_jid && jstatus(jid) != FG) {
        fg_jid = -1;
    }
}

void pgid_stat_bg(int pgid) {
    int jid = pgid2jid(pgid);
    for(int i=0;i<jobs[jid].job_count;i++) {
        if(jobs[jid].jobs[i].stat != DONE) {
            jobs[jid].stat[jobs[jid].jobs[i].stat-1] -= 1;
            jobs[jid].jobs[i].stat = BG;
            jobs[jid].stat[BG-1] += 1;
        }
    }
    if(jid == fg_jid && jstatus(jid) != FG) {
        fg_jid = -1;
    }
}

void update_pgid_status(int pgid,pid_stat_t stat) {
    int jid = pgid2jid(pgid);
    for(int i=0;i<jobs[jid].job_count;i++) {
        jobs[jid].stat[jobs[jid].jobs[i].stat-1] -= 1;
        jobs[jid].jobs[i].stat = stat;
        jobs[jid].stat[stat-1] += 1;
    }
    if(jid == fg_jid && jstatus(jid) != FG) {
        fg_jid = -1;
    }
}

char* jid_cmd(int jid) {
    return jobs[jid].cmd;
}
void update_pid_status(int pid,pid_stat_t stat) {
    int jid = pid2jid(pid);

    for(int i=0;i<jobs[jid].job_count;i++) {
        if(jobs[jid].jobs[i].pid == pid) {
            
            //更新状态
            jobs[jid].stat[jobs[jid].jobs[i].stat-1] -= 1;
            jobs[jid].jobs[i].stat = stat;
            jobs[jid].stat[stat-1] += 1;
        }
    }
    if(jid == fg_jid && jstatus(jid) != FG) {
        fg_jid = -1;
    }
}

pid_stat_t jpstatus(int pid) {
    int jid = pid2jid(pid);
    return jstatus(jid);
}

int get_fg_jid() {
    return fg_jid;
}

void listjobs() {
    for(int i=0;i<MAXJOBS;i++) {
#ifndef DEBUG
        if(jobs[i].pgid != 0) {
#endif
            pid_stat_t stat = jstatus(i);
            printf("[%d] (%d) %d %s : %s\n",i,jobs[i].pgid,stat,stat_to_str(stat),jobs[i].cmd);
#ifndef DEBUG
        }
#endif
    }
}

void do_bgfg(char **argv) 
{
    if(argv[1] == NULL) {
        printf("E: %s command should have 1 arg: jobid or pid.\n",argv[0]);
        return;
    }
    char* endptr;
    pid_stat_t j;
    pid_t pgid = 0;
    int jid = 0;
    if(argv[1][0] == '%') {
        char num[10] = {0};
        strcpy(num,&argv[1][1]);
        jid = strtol(num,&endptr,10);
        if(*endptr != '\0') {
            printf("E: jobid format error.\n");
            return;
        }
        //endptr
        j = jstatus(jid);
        pgid = jid2pgid(pgid);
        if(j == UNDEF) {
            printf("E: No such job: [%d].\n",jid);
            return;
        }
    } else {
        pgid = strtol(argv[1],&endptr,10);
        if(*endptr != '\0') {
            printf("E: pid format error.\n");
            return;
        }
        jid = pid2jid(pgid);
        j = jstatus(jid);
        if(j == UNDEF) {
            printf("E: No such process: %d.\n",pgid);
            return;
        }
    }

    if(strcmp(argv[0],"bg") == 0) {
        pgid_stat_bg(pgid); //更新状态为BG
        if(j == ST) 
            kill(-pgid,SIGCONT);
        
    } else if(strcmp(argv[0],"fg") == 0) {
        pgid_stat_fg(pgid);
        printf("[%d] %d %s: %s\n",jid,pgid, stat_to_str(jstatus(jid)),jid_cmd(jid));
        if(j == ST) {
            kill(-pgid,SIGCONT);
        }
        waitfg(jid);
    }
    return;
}