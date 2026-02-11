#include <file.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
int file_open(char* str) {
    char* buf = str;
    int direction = (*buf == '>'); //1右边(stdout) 0左边(stdin)
    while(*buf && (*buf == ' ' || *buf == '>' || *buf == '<')) buf++;
    char* bend = buf + strlen(buf) - 1;
    while(*bend && (*bend == ' ')) {
        *bend = 0;
        bend--;
    }
    int fd = 0;
    if(direction) {
        int flag = O_WRONLY | O_CREAT;
        if((*str == '>' && *(str+1) == '>')) 
            flag |= O_APPEND;
        fd = open(buf,flag, 0755);
        if(fd < 0) return -1;
    } else {
        fd = open(buf,O_RDONLY);
        if(fd < 0) return -1;
    }
    return fd;
}