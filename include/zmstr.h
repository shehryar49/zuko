// Mutable strings zuko
// kind of like a dynamic array of characters
#ifndef ZMSTR_H
#define ZMSTR_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zobject.h"

struct ZMStr
{
  char* arr;
  size_t size;
  size_t capacity;
};
void ZMStr_init(ZMStr* p)
{
    p->arr = (char*)malloc(sizeof(char)*4);
    p->arr[0] = 0;
    p->capacity = 4;
    p->size = 0;
}
inline void ZMStr_push(ZMStr* p,char val)
{
    if(p -> size >= p->capacity)
    {
      p->arr = (char*)realloc(p->arr,sizeof(char)*p->capacity<<1);
      p->capacity <<= 1; //p->capacity*=2;
    }
    p->arr[p->size++] = val;
 
}
void ZMStr_erase(ZMStr* p,size_t idx)
{
    if(idx < p->size)
    {
      for(size_t i=idx;i<p->size-1;i++)
      {
        p->arr[i] = p->arr[i+1];  
      }
      p->size -= 1;
    }
}
void ZMStr_eraseRange(ZMStr* p,size_t i,size_t j)
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
void ZMStr_resize(ZMStr* p,size_t newSize)
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
    p->arr = (char*)realloc(p->arr,p->capacity* sizeof(char));
    p->size = newSize;
  }
}
bool ZMStr_pop(ZMStr* p,char* val)
{
    if(p->size == 0)
      return false;
    (p->size)--;
    *val = p->arr[p->size];
    return true;
}
inline void ZMStr_fastpop(ZMStr* p,char* val)
{
  *val = p->arr[--p->size];
}
void ZMStr_assign(ZMStr* p,ZMStr* val) // makes deep copy 
{
    if(val->size > 0)
    {
      p->arr = (char*)realloc(p->arr,sizeof(char)*val->capacity);
      memcpy(p->arr,val->arr,val->size*sizeof(char));
      p->capacity = val->capacity;
      p->size = val->size;
    }
    else
    {
        p->size = 0;
        return;
    }
}
void ZMStr_insert(ZMStr* p,size_t idx,char val)
{
  if(idx == p->size) //push
  {
    ZMStr_push(p,val);
    return;
  }
  if(idx < p->size)
  {

    ZMStr_push(p,val);
    size_t i = p->size - 1;
    while(i > idx)
    {
      p->arr[i] = p->arr[i-1];
      i--;
    }  
    p->arr[idx] = val;
  }
}
void ZMStr_insertList(ZMStr* p,size_t idx,ZMStr* substr)
{
  if(substr -> size == 0)
    return;
  if(idx == p->size)
  {
    ZMStr_resize(p,p->size + substr->size);
    memcpy(p->arr + idx,substr->arr,substr->size * sizeof(char));
    return;
  }
  if(idx < p->size)
  {
    // 1 2 3 4 0 0

    size_t i = p->size;
    ZMStr_resize(p,p->size + substr->size);
    memcpy(p->arr + i,p->arr+idx,sizeof(char)*substr->size);
    memcpy(p->arr+idx,substr->arr,sizeof(char)*substr->size);
  }
}
void ZMStr_destroy(ZMStr* p)
{
    free(p->arr);
}

#endif