#ifndef _SHLAB_EVAL
#define _SHLAB_EVAL

#include <config.h>
void unix_error(char *msg);
void pipe_eval(char cmdline[MAXLINE]);
#endif