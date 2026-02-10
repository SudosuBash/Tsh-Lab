#ifndef _SHLAB_STRING_ARRAY
#define _SHLAB_STRING_ARRAY

#include <config.h>
#include <stdlib.h>
typedef struct {
    char* buf;
    size_t len;
} string;

typedef string* pstring;
typedef pstring* ppstring;

typedef struct {
    pstring *strings;
    int arr_length;
} string_array;

typedef string_array* pstring_array;
typedef pstring_array* ppstring_array;

void init_str(ppstring string);
void init_str_with_size(ppstring string,size_t size);
void append_str(pstring src,pstring dst);
void append_c_str(pstring src,char* buf);
void free_str(ppstring string);
void init_str_arr(ppstring_array arr);
void free_str_arr(ppstring_array arr);
int get_str(pstring_array str1, int pos, ppstring recv_str);
size_t split(pstring str1, size_t start, size_t end, ppstring recv_str);
void push_to_arr(pstring_array arr, pstring str);
void append_chr(pstring src,char chr);
#endif