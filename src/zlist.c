#include "zlist.h"
#include "vm.h"

/*#ifdef BUILDING_ZUKO_INTERPRETER
    void* (*ZUKO_ALLOC)(size_t) = vm_alloc_raw;
    void* (*ZUKO_REALLOC)(void*,size_t) = vm_realloc_raw;
#else*/
    void* (*ZUKO_ALLOC)(size_t) = malloc;
    void* (*ZUKO_REALLOC)(void*,size_t) = realloc;
//#endif

void zlist_init(zlist* p)
{
    p->arr = (zobject*)ZUKO_ALLOC(sizeof(zobject)*4);
    p->capacity = 4;
    p->size = 0;
}
void zlist_push(zlist* p,zobject val)
{
    if(p -> size >= p->capacity)
    {
      p->arr = (zobject*)ZUKO_REALLOC(p->arr,sizeof(zobject)*p->capacity<<1);
      p->capacity <<= 1; //p->capacity*=2;
    }
    p->arr[p->size++] = val;
 
}
void zlist_erase(zlist* p,size_t idx)
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
void zlist_erase_range(zlist* p,size_t i,size_t j)
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
void zlist_resize(zlist* p,size_t newSize)
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
    p->arr = (zobject*)ZUKO_REALLOC(p->arr,p->capacity* sizeof(zobject));
    p->size = newSize;
  }
}
bool zlist_pop(zlist* p,zobject* val)
{
    if(p->size == 0)
      return false;
    (p->size)--;
    *val = p->arr[p->size];
    return true;
}

void zlist_assign(zlist* p,zlist* val)
{
    if(val->size > 0)
    {
      p->arr = (zobject*)ZUKO_REALLOC(p->arr,sizeof(zobject)*val->capacity);
      memcpy(p->arr,val->arr,val->size*sizeof(zobject));
      p->capacity = val->capacity;
      p->size = val->size;
    }
    else
    {
        p->size = 0;
        return;
    }
}
void zlist_insert(zlist* p,size_t idx,zobject val)
{
  if(idx == p->size) //push
  {
    zlist_push(p,val);
    return;
  }
  if(idx < p->size)
  {

    zlist_push(p,val);
    size_t i = p->size - 1;
    while(i > idx)
    {
      p->arr[i] = p->arr[i-1];
      i--;
    }  
    p->arr[idx] = val;
  }
}
void zlist_insert_list(zlist* p,size_t idx,zlist* sublist)
{
  if(sublist -> size == 0)
    return;
  if(idx == p->size)
  {
    zlist_resize(p,p->size + sublist->size);
    memcpy(p->arr + idx,sublist->arr,sublist->size * sizeof(zobject));
    return;
  }
  if(idx < p->size)
  {
    // 1 2 3 4 0 0

    size_t i = p->size;
    zlist_resize(p,p->size + sublist->size);
    memcpy(p->arr + i,p->arr+idx,sizeof(zobject)*sublist->size);
    memcpy(p->arr+idx,sublist->arr,sizeof(zobject)*sublist->size);
  }
}
void zlist_destroy(zlist* p)
{
    #ifndef BUILDING_ZUKO_INTERPRETER
        free(p->arr);
    #endif
}

void zlist_fastpop(zlist* p,zobject* val)
{
  *val = p->arr[--p->size];
}

