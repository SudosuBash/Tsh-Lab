#include <job.h>

#include <config.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
static job_t jobs[MAXJOBS] = {0};

int addjob(char cmd[MAXLINE]) {
    for(int i=0;i<MAXJOBS;i++) {
        if(jobs[i].pgid == 0) {
            strcpy(jobs[i].cmd,cmd);
            return i;
        }
    }    
    return -1;
}

void addpid(int jid,int pid,pid_stat_t stat) {
    if(jobs[jid].pgid == 0) {
        jobs[jid].pgid = pid;
    }
    int c = setpgid(pid,jobs[jid].pgid); //设置进程组
    printf("Added job %d pgid %d,status = %d\n",jid,jobs[jid].pgid,c);
    int cnt = jobs[jid].job_count;
    jobs[jid].jobs[cnt].pid = pid;
    jobs[jid].jobs[cnt].stat = stat;
    jobs[jid].jobs[cnt].jid = jid;
    jobs[jid].job_count++;
    jobs[jid].stat[stat-1]++;
}


pid_stat_t jstatus(int jid) {
    job_t j = jobs[jid];
    if(JOB_IS_BACKGROUND(&j) != 0) 
        return BG;
    if(JOB_IS_FOREGROUND(&j) != 0)
        return FG;
    if(JOB_STOPPED_COUNT(&j) == JOB_COUNT(&j))
        return ST;
    if(JOB_DONE_COUNT(&j) == JOB_COUNT(&j)) 
        return DONE;
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
}

pid_stat_t jpstatus(int pid) {
    int jid = pid2jid(pid);
    return jstatus(jid);
}