#ifndef _SHLAB_JOBS
#define _SHLAB_JOBS
#include <config.h>
#include <stdlib.h>

#define JOB_RUNNING_COUNT(job) (((job)->stat[0]) + ((job)->stat[1]))
#define JOB_STOPPED_COUNT(job) ((job)->stat[2])
#define JOB_DONE_COUNT(job) ((job)->stat[3])

#define JOB_COUNT(job) (JOB_RUNNING_COUNT(job) + JOB_STOPPED_COUNT(job) + JOB_DONE_COUNT(job))

#define JOB_IS_RUNNING(job) (JOB_RUNNING_COUNT(job) != 0)
#define JOB_IS_BACKGROUND(job) ((job)->stat[0] != 0)
#define JOB_IS_FOREGROUND(job) ((job)->stat[1] != 0)

typedef enum {
    BG = 1,
    FG = 2,
    ST = 3,
    DONE = 4,
    UNDEF = 0
} pid_stat_t;
typedef struct {
    pid_t pid;
    int jid; //sigchld后,无法获取pgid
    pid_stat_t stat;
} sub_job_t;

typedef struct {
    char cmd[MAXLINE];
    int pgid;
    int stat[4];
    /** 0: BG 1: FG 2: ST 3: DONE*/
    sub_job_t jobs[MAXSUBJOBID];
    int job_count; //任务列表
    
    
} job_t;

int addjob(char cmd[MAXLINE]);

void addpid(int jid,int pid,pid_stat_t stat);
int pgid2jid(pid_t pgid);
pid_stat_t jstatus(int jid);
pid_stat_t jpstatus(int pid);
void update_pid_status(int pid,pid_stat_t stat);
void recyclejob();
void waitfg(int jid);
#endif