#include <cmd_parser.h>
#include <stdlib.h>
#include <string.h>
#define E_NOEND -1

int sep_pipes(const char *cmdline,pstring_array* recv) {
    //管道支持
    static pstring_array arrs;
    init_str_arr(&arrs);

    size_t sz = strlen(cmdline);
    char* buf = cmdline;
    while(*buf && (*buf == ' '))
        buf++;
    while(buf < cmdline + sz) {
        pstring str;
        init_str(&str);
        while(*buf && *buf != '|') {
            append_chr(str,*buf);
            buf++;
        }
        push_to_arr(arrs,str);
        if(*buf && *buf == '|') buf++;
    }
    *recv = arrs;
    return 0;
}

int isbg(const char* cmdline) {
    size_t sl = strlen(cmdline);
    while(cmdline[sl] != '\n' && cmdline[sl] != ' ' && sl >= 0)
        sl--;
    return cmdline[sl] == '&';
}

//用于解析单行命令,不用改
void parseline(const char *cmdline, char **argv) 
{
    static char array[MAXLINE]; /* holds local copy of command line */
    char *buf = array;          /* ptr that traverses command line */
    char *delim;                /* points to first space delimiter */
    int argc;                   /* number of args */
    int bg;                     /* background job? */

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