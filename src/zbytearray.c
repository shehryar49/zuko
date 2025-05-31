#include "zbytearray.h"
#include <stdint.h>

#ifdef BUILDING_ZUKO_INTERPRETER
    #include "vm.h"
#else
    #include "zapi.h"
#endif

void zbytearr_init(zbytearr* p)
{
    p->arr = (uint8_t*)malloc(sizeof(uint8_t)*4); 
    p->capacity = 4;
    p->size = 0;
    vm_increment_allocated(sizeof(uint8_t)*4);
}
 void zbytearr_push(zbytearr* p,uint8_t val)
{
    if(p -> size >= p->capacity)
    {
        vm_increment_allocated(p->capacity);
        p->arr = (uint8_t*)realloc(p->arr,sizeof(uint8_t) * p->capacity<<1);
        p->capacity <<= 1; //p->capacity*=2;
    }
    p->arr[p->size++] = val;
 
}
void zbytearr_erase(zbytearr* p,size_t idx)
{
    if(idx < p->size)
    {
        for(size_t i=idx;i<p->size-1;i++)
            p->arr[i] = p->arr[i+1];  
        p->size -= 1;
    }
}
void zbytearr_erase_range(zbytearr* p,size_t i,size_t j)
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
void zbytearr_resize(zbytearr* p,size_t newSize)
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
        size_t oldcap = p->capacity;
        while(p->capacity < newSize)
            p->capacity <<= 1;
        p->arr = (uint8_t*)realloc(p->arr,p->capacity* sizeof(uint8_t));
        p->size = newSize;
        vm_decrement_allocated(oldcap);
        vm_increment_allocated(p->capacity);
    }
}
bool zbytearr_pop(zbytearr* p,uint8_t* val)
{
    if(p->size == 0)
        return false;
    (p->size)--;
    *val = p->arr[p->size];
    return true;
}
 void zbytearr_fastpop(zbytearr* p,uint8_t* val)
{
    *val = p->arr[--p->size];
}
void zbytearr_assign(zbytearr* p,zbytearr* val)
{
    if(val->size > 0)
    {
        size_t oldcap = p->capacity;
        p->arr = (uint8_t*)realloc(p->arr,sizeof(uint8_t)*val->capacity);
        memcpy(p->arr,val->arr,val->size*sizeof(uint8_t));
        p->capacity = val->capacity;
        p->size = val->size;
        vm_increment_allocated(sizeof(uint8_t) * val -> capacity);
        vm_decrement_allocated(oldcap);
    }
    else
    {
        p->size = 0;
        return;
    }
}
void zbytearr_insert(zbytearr* p,size_t idx,uint8_t val)
{
    if(idx == p->size) //push
    {
        zbytearr_push(p,val);
        return;
    }
    if(idx < p->size)
    {
        zbytearr_push(p,val);
        size_t i = p->size - 1;
        while(i > idx)
        {
            p->arr[i] = p->arr[i-1];
            i--;
        }  
        p->arr[idx] = val;
    }
}
void zbytearr_insert_arr(zbytearr* p,size_t idx,zbytearr* sublist)
{
    if(sublist -> size == 0)
        return;
    if(idx == p->size)
    {
        zbytearr_resize(p,p->size + sublist->size);
        memcpy(p->arr + idx,sublist->arr,sublist->size * sizeof(uint8_t));
        return;
    }
    if(idx < p->size)
    {
        // 1 2 3 4 0 0
        size_t i = p->size;
        zbytearr_resize(p,p->size + sublist->size);
        memcpy(p->arr + i,p->arr+idx,sizeof(uint8_t)*sublist->size);
        memcpy(p->arr+idx,sublist->arr,sizeof(uint8_t)*sublist->size);
    }
}
bool zbytearr_equal(zbytearr* a,zbytearr* b)
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
void zbytearr_destroy(zbytearr* p)
{
    free(p->arr);
    vm_decrement_allocated(p->capacity);
}
