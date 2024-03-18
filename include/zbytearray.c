#include "zbytearray.h"

void ZByteArr_init(ZByteArr* p)
{
    p->arr = (uint8_t*)malloc(sizeof(uint8_t)*4); 
    p->capacity = 4;
    p->size = 0;
}
 void ZByteArr_push(ZByteArr* p,uint8_t val)
{
    if(p -> size >= p->capacity)
    {
      p->arr = (uint8_t*)realloc(p->arr,sizeof(uint8_t)*p->capacity<<1);
      p->capacity <<= 1; //p->capacity*=2;
    }
    p->arr[p->size++] = val;
 
}
void ZByteArr_erase(ZByteArr* p,size_t idx)
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
void ZByteArr_eraseRange(ZByteArr* p,size_t i,size_t j)
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
void ZByteArr_resize(ZByteArr* p,size_t newSize)
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
    p->arr = (uint8_t*)realloc(p->arr,p->capacity* sizeof(uint8_t));
    p->size = newSize;
  }
}
bool ZByteArr_pop(ZByteArr* p,uint8_t* val)
{
    if(p->size == 0)
      return false;
    (p->size)--;
    *val = p->arr[p->size];
    return true;
}
 void ZByteArr_fastpop(ZByteArr* p,uint8_t* val)
{
  *val = p->arr[--p->size];
}
void ZByteArr_assign(ZByteArr* p,ZByteArr* val)
{
    if(val->size > 0)
    {
      p->arr = (uint8_t*)realloc(p->arr,sizeof(uint8_t)*val->capacity);
      memcpy(p->arr,val->arr,val->size*sizeof(uint8_t));
      p->capacity = val->capacity;
      p->size = val->size;
    }
    else
    {
        p->size = 0;
        return;
    }
}
void ZByteArr_insert(ZByteArr* p,size_t idx,uint8_t val)
{
  if(idx == p->size) //push
  {
    ZByteArr_push(p,val);
    return;
  }
  if(idx < p->size)
  {

    ZByteArr_push(p,val);
    size_t i = p->size - 1;
    while(i > idx)
    {
      p->arr[i] = p->arr[i-1];
      i--;
    }  
    p->arr[idx] = val;
  }
}
void ZByteArr_insertArr(ZByteArr* p,size_t idx,ZByteArr* sublist)
{
  if(sublist -> size == 0)
    return;
  if(idx == p->size)
  {
    ZByteArr_resize(p,p->size + sublist->size);
    memcpy(p->arr + idx,sublist->arr,sublist->size * sizeof(uint8_t));
    return;
  }
  if(idx < p->size)
  {
    // 1 2 3 4 0 0

    size_t i = p->size;
    ZByteArr_resize(p,p->size + sublist->size);
    memcpy(p->arr + i,p->arr+idx,sizeof(uint8_t)*sublist->size);
    memcpy(p->arr+idx,sublist->arr,sizeof(uint8_t)*sublist->size);
  }
}
bool ZByteArr_equal(ZByteArr* a,ZByteArr* b)
{
  if(a->size != b->size)
    return false;
  for(size_t i=0;i<a->size;i++)
  {
    if(a->arr[i] != b->arr[i])
      return false;
  }
  return true;
}
void ZByteArr_destroy(ZByteArr* p)
{
    free(p->arr);
}