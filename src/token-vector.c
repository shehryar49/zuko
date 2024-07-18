#include "token-vector.h"

void token_vector_init (token_vector* p)
{
    p->arr = (token*)malloc(sizeof(token)*4);  
    p->capacity = 4;
    p->size = 0;
}
void token_vector_push(token_vector* p,token val)
{
    if(p -> size >= p->capacity)
    {
      p->arr = (token*)realloc(p->arr,sizeof(token)*p->capacity<<1);
      p->capacity <<= 1; //p->capacity*=2;
    }
    p->arr[p->size++] = val;
 
}
bool token_vector_pop(token_vector* p,token* val)
{
    if(p->size == 0)
      return false;
    (p->size)--;
    *val = p->arr[p->size];
    return true;
}
void token_vector_assign(token_vector* p,token_vector* val)
{
    if(val->size > 0)
    {
      p->arr = (token*)realloc(p->arr,sizeof(token)*val->capacity);
      memcpy(p->arr,val->arr,val->size*sizeof(token));
      p->capacity = val->capacity;
      p->size = val->size;
    }
    else
    {
        p->size = 0;
        return;
    }
}
int token_vector_search(token_vector* p,token val)
{
    for(size_t i=0;i<p->size;i++)
    {
        if(p->arr[i].type == val.type && strcmp(p->arr[i].content,val.content) == 0)
            return (int)i;
    }
    return -1;
}
void token_vector_destroy(token_vector* p)
{
    free(p->arr);
}