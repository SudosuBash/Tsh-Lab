#ifndef _SHLAB_CMD_PARSER
#define _SHLAB_CMD_PARSER

#include <p_string.h>
#include <config.h>

//以下的东西是已经帮你写好了的
void parseline(char *cmdline, char **argv); 
int sep_pipes(char *cmdline,pstring_array* recv);
int isbg(char* cmdline);
int builtin_cmd(char **argv);
int builtin_cmd_not_fork(char** argv,int i);
#endif