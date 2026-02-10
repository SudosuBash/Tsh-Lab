#include <p_string.h>
#include <stdlib.h>
#include <config.h>
#include <string.h>

#define E_OUT_OF_RANGE -1

void init_str(ppstring string) {
    *string = (pstring)calloc(1,sizeof(string));
    (*string)->len = 0; //字符串长度
    (*string)->buf = (char*)calloc(1,1); 
}

void init_str_with_size(ppstring string,size_t size) {
    *string = (pstring)calloc(1,sizeof(string));
    (*string)->len = size; //字符串长度
    (*string)->buf = (char*)calloc(1,size + 1); 
}

void append_str(pstring src,pstring dst) {
    size_t l = src->len + dst->len;
    size_t srcl = src->len;
    src->len = l;
    src->buf = (char*) realloc(src->buf, src->len + 1);
    strcpy(src->buf + srcl, dst->buf);
    src->buf[src->len] = 0;
}

void append_chr(pstring src,char chr) {
    size_t l = src->len + 1;
    size_t srcl = src->len;
    src->len = l;
    src->buf = (char*) realloc(src->buf, src->len + 1);
    src->buf[srcl] = chr;
    src->buf[src->len] = 0;
}
void append_c_str(pstring src,char* buf) {
    size_t l = src->len;
    src->len += strlen(buf);
    src->buf = (char*)realloc(src->buf,src->len + 1);
    strcpy(src->buf+l,buf);
    src->buf[src->len] = 0;//最后一个设置为0
}

void free_str(ppstring string) {
    free((*string)->buf);
    free(*string);
    *string = NULL;
}

void init_str_arr(ppstring_array arr) {
    *arr = (pstring_array)calloc(1,sizeof(string_array));
    (*arr)->arr_length = 0;
    (*arr)->strings = NULL;
}


void free_str_arr(ppstring_array arr) {
    pstring_array parr = *arr;
    for(int i=0;i<parr->arr_length;i++) {
        free_str(&parr->strings[i]);
    }
    free(parr);
    *arr = NULL;
}

int get_str(pstring_array str1, int pos, ppstring recv_str) {
    if(pos >= str1->arr_length) {
        return E_OUT_OF_RANGE;
    }
    *recv_str = str1->strings[pos];
    return 0;
}

size_t split(pstring str1, size_t start, size_t end, ppstring recv_str) {
    end = min(end,str1->len); //限制死
    start = max(start,0);
    if(start > end) return 0;

    pstring str;
    init_str_with_size(&str,end - start + 1);

    memcpy(str->buf,str1->buf+start, end - start + 1);
    *recv_str = str;
    return end - start + 1;
}

void push_to_arr(pstring_array arr, pstring str) {
    if(arr->arr_length == 0) {
        arr->strings = (ppstring)calloc(1,sizeof(pstring));
        arr->arr_length = 1;
    } else {
        arr->arr_length ++;
        arr->strings = (ppstring)realloc(arr->strings,sizeof(pstring) * arr->arr_length);
        // arr->strings = (ppstring)realloc(arr->strings,sizeof(pstring));
        //又是内存分配的bug
        //大问题, 字符串解析都错了, 要真实工程项目敢出现这个错误
        
    }
    arr->strings[arr->arr_length-1] = str;
}