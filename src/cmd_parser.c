#include <cmd_parser.h>
#include <stdlib.h>
#include <job.h>
#include <string.h>
#include <stdio.h>

//简单重定向
char sep_redirect(char* cmdline,pstring_array* recv) {
    pstring_array arrs;
    init_str_arr(&arrs);
    char* buf = cmdline;
    size_t sz = strlen(cmdline);
    while(*buf && (*buf == ' ')) buf++;
    while(buf < cmdline + sz) {
        pstring str;
        init_str(&str);
        char st_chr = *buf;
        if(*buf && buf != cmdline) {
            append_chr(str,*buf); //加进去
            buf++;
            char prv = *(buf-1);
            if(prv == '>' &&  prv == buf[0]) {
                append_chr(str,*buf); //加进去
                buf++;
            } //目前就解析 '>>'       
        }
        int allBlank = 1;
        while(*buf && *buf != '>' && *buf != '<') {
            if(*buf != ' ') allBlank = 0;
            append_chr(str,*buf);
            buf++;
        }
        push_to_arr(arrs,str);
        if(allBlank == 1) {
            *recv = arrs;
            return st_chr;
        }
    }

    *recv = arrs;
    return 0;
}

//管道支持
char sep_pipes(char *cmdline,pstring_array* recv) {

    pstring_array arrs;
    init_str_arr(&arrs);

    size_t sz = strlen(cmdline);
    char* buf = (char*)cmdline;
    while(*buf && (*buf == ' '))
        buf++;
    while(buf < cmdline + sz) {
        int allBlank = 1;
        pstring str;
        init_str(&str);
        while(*buf && *buf != '|') {
            if(*buf != ' ') allBlank = 0;
            append_chr(str,*buf);
            buf++;
        }
        if(allBlank) {
            *recv = arrs;
            return *buf; //parse error
        }
        push_to_arr(arrs,str);
        if(*buf && *buf == '|') buf++;
    }
    *recv = arrs;
    return 0;
}

int isbg(char* cmdline) {
    size_t sl = strlen(cmdline);
    while((cmdline[sl] == '\n' || cmdline[sl] == ' ' || cmdline[sl] == 0) && sl >= 0)
        sl--;
    if(cmdline[sl] == '&') {
        cmdline[sl] = 0;
        return 1;
    }
    return 0;
}

//用于解析单行命令,不用改
void parseline(char *cmdline, char **argv) 
{
    static char array[MAXLINE]; /* holds local copy of command line */
    char *buf = array;          /* ptr that traverses command line */
    char *delim;                /* points to first space delimiter */
    int argc;                   /* number of args */

    strcpy(buf, cmdline);
    size_t sl = strlen(buf)-1;
    while(sl > 0 && (buf[sl] == ' ' || buf[sl] == '\n')) {
        
        buf[sl] = 0;
        sl--;
    }
    buf[++sl] = ' ';
    // buf[strlen(buf)-1] = ' ';  /* replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* ignore leading spaces */
	buf++;

    /* Build the argv list */
    argc = 0;
    if (*buf == '\'') {
	buf++;
	delim = strchr(buf, '\'');
    }
    else {
	delim = strchr(buf, ' ');
    }

    while (delim) {
	argv[argc++] = buf;
	*delim = '\0';
	buf = delim + 1;
	while (*buf && (*buf == ' ')) /* ignore spaces */
	       buf++;

	if (*buf == '\'') {
	    buf++;
	    delim = strchr(buf, '\'');
	}
	else {
	    delim = strchr(buf, ' ');
	}
    }
    argv[argc] = NULL;
    
}

//此类命令不能丢在fork和管道中
int builtin_cmd_not_fork(char** argv,int i) {
    
    if(strcmp(argv[0],"bg") == 0 || strcmp(argv[0],"fg") == 0) {
        if(i > 0) return -1;
        do_bgfg(argv);
        return 1;
    } else if(strcmp(argv[0],"quit") == 0) {
        if(i > 0) return -1;
        exit(0);
    }
    return 0;
}
int builtin_cmd(char **argv) //此类命令可以丢在管道中
{
    if(strcmp(argv[0],"jobs") == 0) {
        listjobs();
        return 1;
    }
    return 0;     /* not a builtin command */
}
