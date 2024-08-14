#ifndef ZUKO_PTR_VECTOR_H_
#define ZUKO_PTR_VECTOR_H_

#ifdef __cplusplus
extern "C"{
#endif
typedef struct Node Node;
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct ptr_vector
{
    void** arr;
    size_t size;
    size_t capacity;
}ptr_vector;

void ptr_vector_init (ptr_vector* p);
void ptr_vector_push(ptr_vector* p,void* val);
bool ptr_vector_pop(ptr_vector* p,void** val);
void ptr_vector_assign(ptr_vector* p,ptr_vector* val); //makes deep copy
int ptr_vector_search(ptr_vector* p,void* val);
void ptr_vector_destroy(ptr_vector* p);

#ifdef __cplusplus
}
#endif

#endif