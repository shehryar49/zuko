#ifndef ZUKO_STR_VECTOR_H_
#define ZUKO_STR_VECTOR_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct str_vector
{
    char** arr;
    size_t size;
    size_t capacity;
}str_vector;

void str_vector_init (str_vector* p);
void str_vector_push(str_vector* p,char* val);
void str_vector_erase(str_vector* p,size_t idx);
void str_vector_erase_range(str_vector* p,size_t i,size_t j);
void str_vector_resize(str_vector* p,size_t newSize);
bool str_vector_pop(str_vector* p,char** val);
void str_vector_assign(str_vector* p,str_vector* val);
void str_vector_insert(str_vector* p,size_t idx,char* val);
void str_vector_insert_vector(str_vector* p,size_t idx,str_vector* sublist);
int str_vector_search(str_vector* p,const char* val);
void str_vector_destroy(str_vector* p);

#ifdef __cplusplus
}
#endif

#endif