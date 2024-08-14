#include "nodeptr_vec.h"

void nodeptr_vector_init (nodeptr_vector* p)
{
    p->arr = (Node**)malloc(sizeof(Node*)*4);  
    p->capacity = 4;
    p->size = 0;
}
void nodeptr_vector_push(nodeptr_vector* p,Node* val)
{
    if(p -> size >= p->capacity)
    {
      p->arr = (Node**)realloc(p->arr,sizeof(Node*)*p->capacity<<1);
      p->capacity <<= 1; //p->capacity*=2;
    }
    p->arr[p->size++] = val;
 
}
bool nodeptr_vector_pop(nodeptr_vector* p,Node** val)
{
    if(p->size == 0)
      return false;
    (p->size)--;
    *val = p->arr[p->size];
    return true;
}
void nodeptr_vector_assign(nodeptr_vector* p,nodeptr_vector* val)
{
    if(val->size > 0)
    {
      p->arr = (Node**)realloc(p->arr,sizeof(Node*)*val->capacity);
      memcpy(p->arr,val->arr,val->size*sizeof(Node*));
      p->capacity = val->capacity;
      p->size = val->size;
    }
    else
    {
        p->size = 0;
        return;
    }
}
int nodeptr_vector_search(nodeptr_vector* p,Node* val)
{
    for(size_t i=0;i<p->size;i++)
    {
        if(p->arr[i] == val)
            return (int)i;
    }
    return -1;
}
void nodeptr_vector_destroy(nodeptr_vector* p)
{
    free(p->arr);
}