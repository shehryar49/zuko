#include "ptr-vector.h"

void ptr_vector_init (ptr_vector* p)
{
    p->arr = (void**)malloc(sizeof(void*)*4);  
    p->capacity = 4;
    p->size = 0;
}
void ptr_vector_push(ptr_vector* p,void* val)
{
    if(p -> size >= p->capacity)
    {
      p->arr = (void**)realloc(p->arr,sizeof(void*)*p->capacity<<1);
      p->capacity <<= 1; //p->capacity*=2;
    }
    p->arr[p->size++] = val;
 
}
bool ptr_vector_pop(ptr_vector* p,void** val)
{
    if(p->size == 0)
      return false;
    (p->size)--;
    *val = p->arr[p->size];
    return true;
}
void ptr_vector_assign(ptr_vector* p,ptr_vector* val)
{
    if(val->size > 0)
    {
      p->arr = (void**)realloc(p->arr,sizeof(void*)*val->capacity);
      memcpy(p->arr,val->arr,val->size*sizeof(void*));
      p->capacity = val->capacity;
      p->size = val->size;
    }
    else
    {
        p->size = 0;
        return;
    }
}
int ptr_vector_search(ptr_vector* p,void* val)
{
    for(size_t i=0;i<p->size;i++)
    {
        if(p->arr[i] == val)
            return (int)i;
    }
    return -1;
}
void ptr_vector_destroy(ptr_vector* p)
{
    free(p->arr);
}