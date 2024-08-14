#ifndef ZUKO_PAIR_VECTOR_H_
#define ZUKO_PAIR_VECTOR_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct zpair
{
    int32_t x;
    int32_t y;
}zpair;

typedef struct pair_vector
{
    zpair* arr;
    size_t size;
    size_t capacity;
}pair_vector;

void pair_vector_init (pair_vector* p);
void pair_vector_push(pair_vector* p,int32_t x,int32_t y);
void pair_vector_destroy(pair_vector* p);


#ifdef __cplusplus
}
#endif

#endif