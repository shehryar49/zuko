#include "pair-vector.h"


void pair_vector_init (pair_vector* p)
{
    p->arr = (zpair*)malloc(sizeof(zpair)*4);  
    p->capacity = 4;
    p->size = 0;
}
void pair_vector_push(pair_vector* p,int32_t x,int32_t y)
{
    zpair val = {.x = x,.y = y};

    if(p -> size >= p->capacity)
    {
      p->arr = (zpair*)realloc(p->arr,sizeof(zpair)*p->capacity<<1);
      p->capacity <<= 1; //p->capacity*=2;
    }
    p->arr[p->size++] = val;
 
}
void pair_vector_destroy(pair_vector* p)
{
    free(p->arr);
}