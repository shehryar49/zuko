#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C"{
#endif
// VEC_TYPE and VEC_TYPENAME are macros defined by the file that includes this header
#define CONCAT(a,b) a##_##b
#define CONCAT1(a,b) CONCAT(a,b)
#define VECNAME CONCAT1(VEC_TYPENAME,vector)
//#define STRING(x)
//#pragma message(VECNAME)

typedef struct VECNAME
{
    VEC_TYPE* arr;
    size_t size;
    size_t capacity;
}VECNAME;

extern inline void CONCAT1(VECNAME,init) (VECNAME* p)
{
    p->arr = (VEC_TYPE*)malloc(sizeof(VEC_TYPE)*4);  
    p->capacity = 4;
    p->size = 0;
}
extern inline void CONCAT1(VECNAME,push)(VECNAME* p,VEC_TYPE val)
{
    if(p -> size >= p->capacity)
    {
      p->arr = (VEC_TYPE*)realloc(p->arr,sizeof(VEC_TYPE)*p->capacity<<1);
      p->capacity <<= 1; //p->capacity*=2;
    }
    p->arr[p->size++] = val;
 
}
extern inline void CONCAT1(VECNAME,erase)(VECNAME* p,size_t idx)
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
extern inline void CONCAT1(VECNAME,erase_range)(VECNAME* p,size_t i,size_t j)
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
extern inline void CONCAT1(VECNAME,resize)(VECNAME* p,size_t newSize)
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
    p->arr = (VEC_TYPE*)realloc(p->arr,p->capacity* sizeof(VEC_TYPE));
    p->size = newSize;
  }
}
extern inline bool CONCAT1(VECNAME,pop)(VECNAME* p,VEC_TYPE* val)
{
    if(p->size == 0)
      return false;
    (p->size)--;
    *val = p->arr[p->size];
    return true;
}
extern inline void CONCAT1(VECNAME,assign)(VECNAME* p,VECNAME* val)
{
    if(val->size > 0)
    {
      p->arr = (VEC_TYPE*)realloc(p->arr,sizeof(VEC_TYPE)*val->capacity);
      memcpy(p->arr,val->arr,val->size*sizeof(VEC_TYPE));
      p->capacity = val->capacity;
      p->size = val->size;
    }
    else
    {
        p->size = 0;
        return;
    }
}
extern inline void CONCAT1(VECNAME,insert)(VECNAME* p,size_t idx,VEC_TYPE val)
{
  if(idx == p->size) //push
  {
    CONCAT1(VECNAME,push)(p,val);
    return;
  }
  if(idx < p->size)
  {

    CONCAT1(VECNAME,push)(p,val);
    size_t i = p->size - 1;
    while(i > idx)
    {
      p->arr[i] = p->arr[i-1];
      i--;
    }  
    p->arr[idx] = val;
  }
}
extern inline void CONCAT1(VECNAME,insert_vector)(VECNAME* p,size_t idx,VECNAME* sublist)
{
  if(sublist -> size == 0)
    return;
  if(idx == p->size)
  {
    CONCAT1(VECNAME,resize)(p,p->size + sublist->size);
    memcpy(p->arr + idx,sublist->arr,sublist->size * sizeof(VEC_TYPE));
    return;
  }
  if(idx < p->size)
  {
    // 1 2 3 4 0 0

    size_t i = p->size;
    CONCAT1(VECNAME,resize)(p,p->size + sublist->size);
    memcpy(p->arr + i,p->arr+idx,sizeof(VEC_TYPE)*sublist->size);
    memcpy(p->arr+idx,sublist->arr,sizeof(VEC_TYPE)*sublist->size);
  }
}

#ifdef __cplusplus
}
#endif