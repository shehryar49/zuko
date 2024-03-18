#include "zlist.h"

void ZList_init(ZList* p)
{
    p->arr = (ZObject*)malloc(sizeof(ZObject)*4);
    p->arr[0].type = 'n';
    p->arr[1].type = 'n';
    p->arr[2].type = 'n';
    p->arr[3].type = 'n';
    
    p->capacity = 4;
    p->size = 0;
}
void ZList_push(ZList* p,ZObject val)
{
    if(p -> size >= p->capacity)
    {
      p->arr = (ZObject*)realloc(p->arr,sizeof(ZObject)*p->capacity<<1);
      p->capacity <<= 1; //p->capacity*=2;
    }
    p->arr[p->size++] = val;
 
}
void ZList_erase(ZList* p,size_t idx)
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
void ZList_eraseRange(ZList* p,size_t i,size_t j)
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
void ZList_resize(ZList* p,size_t newSize)
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
    p->arr = (ZObject*)realloc(p->arr,p->capacity* sizeof(ZObject));
    p->size = newSize;
  }
}
bool ZList_pop(ZList* p,ZObject* val)
{
    if(p->size == 0)
      return false;
    (p->size)--;
    *val = p->arr[p->size];
    return true;
}
void ZList_fastpop(ZList* p,ZObject* val)
{
  *val = p->arr[--p->size];
}
void ZList_assign(ZList* p,ZList* val)
{
    if(val->size > 0)
    {
      p->arr = (ZObject*)realloc(p->arr,sizeof(ZObject)*val->capacity);
      memcpy(p->arr,val->arr,val->size*sizeof(ZObject));
      p->capacity = val->capacity;
      p->size = val->size;
    }
    else
    {
        p->size = 0;
        return;
    }
}
void ZList_insert(ZList* p,size_t idx,ZObject val)
{
  if(idx == p->size) //push
  {
    ZList_push(p,val);
    return;
  }
  if(idx < p->size)
  {

    ZList_push(p,val);
    size_t i = p->size - 1;
    while(i > idx)
    {
      p->arr[i] = p->arr[i-1];
      i--;
    }  
    p->arr[idx] = val;
  }
}
void ZList_insertList(ZList* p,size_t idx,ZList* sublist)
{
  if(sublist -> size == 0)
    return;
  if(idx == p->size)
  {
    ZList_resize(p,p->size + sublist->size);
    memcpy(p->arr + idx,sublist->arr,sublist->size * sizeof(ZObject));
    return;
  }
  if(idx < p->size)
  {
    // 1 2 3 4 0 0

    size_t i = p->size;
    ZList_resize(p,p->size + sublist->size);
    memcpy(p->arr + i,p->arr+idx,sizeof(ZObject)*sublist->size);
    memcpy(p->arr+idx,sublist->arr,sizeof(ZObject)*sublist->size);
  }
}
void ZList_destroy(ZList* p)
{
    free(p->arr);
}