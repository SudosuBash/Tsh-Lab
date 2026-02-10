#ifndef _SHLAB_CMD_PARSER
#define _SHLAB_CMD_PARSER

#include <p_string.h>
#include <config.h>

//以下的东西是已经帮你写好了的
void parseline(const char *cmdline, char **argv); 
int sep_pipes(const char *cmdline,pstring_array* recv);
int isbg(const char* cmdline);
#endif