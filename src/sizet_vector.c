#include "sizet_vector.h"
#include <string.h>

void sizet_vector_init(sizet_vector* p)
{
    p->arr = (size_t*)malloc(sizeof(size_t)*4);  
    p->capacity = 4;
    p->size = 0;
}
void sizet_vector_push(sizet_vector* p,size_t val)
{
    if(p -> size >= p->capacity)
    {
      p->arr = (size_t*)realloc(p->arr,sizeof(size_t)*p->capacity<<1);
      p->capacity <<= 1; //p->capacity*=2;
    }
    p->arr[p->size++] = val;
 
}
void sizet_vector_erase(sizet_vector* p,size_t idx)
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
void sizet_vector_erase_range(sizet_vector* p,size_t i,size_t j)
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
void sizet_vector_resize(sizet_vector* p,size_t newSize)
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
    p->arr = (size_t*)realloc(p->arr,p->capacity* sizeof(size_t));
    p->size = newSize;
  }
}
bool sizet_vector_pop(sizet_vector* p,size_t* val)
{
    if(p->size == 0)
      return false;
    (p->size)--;
    *val = p->arr[p->size];
    return true;
}

void sizet_vector_assign(sizet_vector* p,sizet_vector* val)
{
    if(val->size > 0)
    {
      p->arr = (size_t*)realloc(p->arr,sizeof(size_t)*val->capacity);
      memcpy(p->arr,val->arr,val->size*sizeof(size_t));
      p->capacity = val->capacity;
      p->size = val->size;
    }
    else
    {
        p->size = 0;
        return;
    }
}
void sizet_vector_insert(sizet_vector* p,size_t idx,size_t val)
{
  if(idx == p->size) //push
  {
    sizet_vector_push(p,val);
    return;
  }
  if(idx < p->size)
  {

    sizet_vector_push(p,val);
    size_t i = p->size - 1;
    while(i > idx)
    {
      p->arr[i] = p->arr[i-1];
      i--;
    }  
    p->arr[idx] = val;
  }
}
void sizet_vector_insert_list(sizet_vector* p,size_t idx,sizet_vector* sublist)
{
  if(sublist -> size == 0)
    return;
  if(idx == p->size)
  {
    sizet_vector_resize(p,p->size + sublist->size);
    memcpy(p->arr + idx,sublist->arr,sublist->size * sizeof(size_t));
    return;
  }
  if(idx < p->size)
  {
    // 1 2 3 4 0 0

    size_t i = p->size;
    sizet_vector_resize(p,p->size + sublist->size);
    memcpy(p->arr + i,p->arr+idx,sizeof(size_t)*sublist->size);
    memcpy(p->arr+idx,sublist->arr,sizeof(size_t)*sublist->size);
  }
}
void sizet_vector_destroy(sizet_vector* p)
{
    free(p->arr);
}