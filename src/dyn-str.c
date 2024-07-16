#include "dyn-str.h"
#include <string.h>

void dyn_str_init(dyn_str* p)
{
    p->arr = malloc(sizeof(char)*4);
    p->arr[0] = 0;
    p->length = 0;
    p->capacity = 4;
}

void dyn_str_push(dyn_str* p,char ch)
{
    if(p->length + 1 >= p->capacity)
    {
        p->arr = realloc(p->arr,p->capacity*2);
        p->capacity *= 2;
    }
    p->arr[p->length] = ch;
    p->arr[p->length+1] = 0;
    p->length++;
}

bool dyn_str_pop(dyn_str* p,char* c)
{
    if(p->length != 0)
    {
        *c = p->arr[--(p->length)];
        p->arr[p->length] = 0;
        return true;
    }
    return false;
}
void dyn_str_prepend_cstr(dyn_str* p,const char* str)
{
    size_t len = strlen(str);
    if(p->length+1+len >= p->capacity)
    {
        while(p->length+1+len >= p->capacity)
            p->capacity *= 2;
        p->arr = realloc(p->arr,p->capacity);
    }
    memcpy(p->arr+len, p->arr,p->length);
    memcpy(p->arr,str, len);
    p->arr[p->length + len] = 0;
}
void dyn_str_destroy(dyn_str* p)
{
    free(p->arr);
}