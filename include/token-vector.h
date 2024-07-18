#ifndef ZUKO_TOKEN_VECTOR_H_
#define ZUKO_TOKEN_VECTOR_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "token.h"

typedef struct token_vector
{
    token* arr;
    size_t size;
    size_t capacity;
}token_vector;

void token_vector_init (token_vector* p);
void token_vector_push(token_vector* p,token val);
bool token_vector_pop(token_vector* p,token* val);
void token_vector_assign(token_vector* p,token_vector* val); //makes deep copy
int token_vector_search(token_vector* p,token val);
void token_vector_destroy(token_vector* p);

#ifdef __cplusplus
}
#endif

#endif