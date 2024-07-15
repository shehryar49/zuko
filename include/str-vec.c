#include "str-vec.h"

void str_vector_init (str_vector* p)
{
    p->arr = (char**)malloc(sizeof(char*)*4);  
    p->capacity = 4;
    p->size = 0;
}
void str_vector_push(str_vector* p,char* val)
{
    if(p -> size >= p->capacity)
    {
      p->arr = (char**)realloc(p->arr,sizeof(char*)*p->capacity<<1);
      p->capacity <<= 1; //p->capacity*=2;
    }
    p->arr[p->size++] = val;
 
}
void str_vector_erase(str_vector* p,size_t idx)
{
    if(idx < p->size)
    {
      //1 2 3 4 5
      /*
        1 3 4 5 5
      */
      //left shift arr starting from idx to size-1
      for(size_t i=idx;i<p->size-1;i++)
      {
        p->arr[i] = p->arr[i+1];  
      }
      p->size -= 1;
    }
}
void str_vector_erase_range(str_vector* p,size_t i,size_t j)
{
  if(i <= j && i < p->size && j<p->size)
  {
    if( j == p->size - 1)
    {
      p->size = i;
      return;
    }
    size_t l = i;
    size_t m = j+1;
    while(m < p->size)
    {
      p->arr[l] = p->arr[m];
      l++;
      m++;
    }
    p -> size -= j-i+1;

  }
}
void str_vector_resize(str_vector* p,size_t newSize)
{
  if(newSize <= p->size)
  {
    p->size = newSize;
    return;
  }
  if(newSize <= p->capacity)
  {
    p->size = newSize;
  }
  else
  {
    while(p->capacity < newSize)
      p->capacity <<= 1;
    p->arr = (char**)realloc(p->arr,p->capacity* sizeof(char*));
    p->size = newSize;
  }
}
bool str_vector_pop(str_vector* p,char** val)
{
    if(p->size == 0)
      return false;
    (p->size)--;
    *val = p->arr[p->size];
    return true;
}
void str_vector_assign(str_vector* p,str_vector* val)
{
    if(val->size > 0)
    {
      p->arr = (char**)realloc(p->arr,sizeof(char*)*val->capacity);
      memcpy(p->arr,val->arr,val->size*sizeof(char*));
      p->capacity = val->capacity;
      p->size = val->size;
    }
    else
    {
        p->size = 0;
        return;
    }
}
void str_vector_insert(str_vector* p,size_t idx,char* val)
{
  if(idx == p->size) //push
  {
    str_vector_push(p,val);
    return;
  }
  if(idx < p->size)
  {

    str_vector_push(p,val);
    size_t i = p->size - 1;
    while(i > idx)
    {
      p->arr[i] = p->arr[i-1];
      i--;
    }  
    p->arr[idx] = val;
  }
}
void str_vector_insert_vector(str_vector* p,size_t idx,str_vector* sublist)
{
  if(sublist -> size == 0)
    return;
  if(idx == p->size)
  {
    str_vector_resize(p,p->size + sublist->size);
    memcpy(p->arr + idx,sublist->arr,sublist->size * sizeof(char*));
    return;
  }
  if(idx < p->size)
  {
    // 1 2 3 4 0 0

    size_t i = p->size;
    str_vector_resize(p,p->size + sublist->size);
    memcpy(p->arr + i,p->arr+idx,sizeof(char*)*sublist->size);
    memcpy(p->arr+idx,sublist->arr,sizeof(char*)*sublist->size);
  }
}
int str_vector_search(str_vector* p,const char* val)
{
    for(size_t i=0;i<p->size;i++)
    {
        if(p->arr[i] == val || strcmp(p->arr[i],val) == 0)
            return (int)i;
    }
    return -1;
}
void str_vector_destroy(str_vector* p)
{
    free(p->arr);
}