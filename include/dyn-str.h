#ifndef ZUKO_DYNAMIC_STR_H_
#define ZUKO_DYNAMIC_STR_H_
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
  extern "C"{
#endif

typedef struct dyn_str
{
    char* arr;
    size_t length; //length+1 bytes are being used always
    size_t capacity;
}dyn_str;

void dyn_str_init(dyn_str* p);
void dyn_str_push(dyn_str* p,char val);
bool dyn_str_pop(dyn_str* p,char* ptr);
void dyn_str_prepend(dyn_str*,const char*);
void dyn_str_append(dyn_str*,const char*);
void dyn_str_destroy(dyn_str* p);

#ifdef __cplusplus
}
#endif
#endif
