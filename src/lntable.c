#include "lntable.h"
#include "byte_src.h"
#include "zdict.h"
#include <stdio.h>
static size_t hash_key(size_t a, size_t M)
{
    return ((a * 31) & (M-1));
}
void lntable_init(lntable* h)
{
  h->table = (lntable_slot*)malloc(sizeof(lntable_slot)*4);
  h->size = 0;
  h->capacity = 4;
  for(int i=0;i<4;i++)
    h->table[i].stat = LN_EMPTY;
}
void lntable_set(lntable* h,size_t key,byte_src val)
{
  size_t idx = hash_key(key,h->capacity);
  int i = 1;
  bool reassigned = false;
  while(h->table[idx].stat == LN_OCCUPIED)
  {
    if(h->table[idx].stat == LN_OCCUPIED && h->table[idx].key == key) //same key, just reassign value
    {
      reassigned = true;
      break;
    }
    idx = (idx + i*i)  & (h->capacity - 1);
    i++;
  }
  h->table[idx].key = key;
  h->table[idx].val = val;
  h->table[idx].stat = LN_OCCUPIED;
  if(!reassigned)
    (h->size)++;
  if(h->size  == 0.75f* h->capacity) //rehash
  {
    printf("rehashing(1)\n");
    lntable_slot* newArr = (lntable_slot*)malloc(sizeof(lntable_slot)*(h->capacity*2));
    for(size_t i=0;i<h->capacity*2;i++)
      newArr[i].stat = LN_EMPTY;
    lntable_slot* old = h->table;
    size_t oldcap = h->capacity;
    h->size = 0;
    h->table = newArr;
    h->capacity = h->capacity*2;
    for(size_t i=0;i<oldcap;i++)
    {
      if(old[i].stat == LN_OCCUPIED)
        lntable_set(h,old[i].key,old[i].val);
    }
    free(old);
  }
}
void lntable_emplace(lntable* h,size_t key,byte_src val)
{
  size_t idx = hash_key(key,h->capacity);
  int i = 1;
  while(h->table[idx].stat == LN_OCCUPIED)
  {
    if(h->table[idx].key == key) //same key, don't reassign value
        return;
    idx = (idx + i*i)  & (h->capacity - 1);
    i++;
  }
  
  h->table[idx].key = key;
  h->table[idx].val = val;
  h->table[idx].stat = LN_OCCUPIED;
  (h->size)++;
  if(h->size  == 0.75f* h->capacity) //rehash
  {
    lntable_slot* newArr = (lntable_slot*)malloc(sizeof(lntable_slot)*(h->capacity*2));
    for(size_t i=0;i<h->capacity*2;i++)
      newArr[i].stat = LN_EMPTY;
    lntable_slot* old = h->table;
    size_t oldcap = h->capacity;
    h->size = 0;
    h->table = newArr;
    h->capacity = h->capacity*2;
    for(size_t i=0;i<oldcap;i++)
    {
        if(old[i].stat == LN_OCCUPIED)
            lntable_set(h,old[i].key,old[i].val);
    }
    free(old);
  }
}
bool lntable_get(lntable* h,size_t key,byte_src* val)
{
  size_t idx = hash_key(key,h->capacity);
  int i = 1;
  while(h->table[idx].stat != LN_EMPTY)
  {
    if(h->table[idx].stat == LN_OCCUPIED && h->table[idx].key == key)
    {
      *val = h->table[idx].val;
      return true;
    }
    idx = (idx + i*i) & (h->capacity - 1);
    i++;
  }
  return false;
}
bool lntable_erase(lntable* h,size_t key)
{
  size_t idx = hash_key(key,h->capacity);
  int i = 1;
  while(h->table[idx].stat != LN_EMPTY)
  {
    if(h->table[idx].key == key)
    {
      h->table[idx].stat = LN_DELETED;
      h->size--;
      return true;
    }
    idx = (idx + i*i) & (h->capacity - 1);
    i++;
  }
  return false;
}

void lntable_assign(lntable* h,lntable* other) // makes deep copy
{
  for(size_t idx=0;idx<other->capacity;idx++)
  {
    if(other->table[idx].stat != LN_OCCUPIED)
      continue;
    lntable_emplace(h,other->table[idx].key,other->table[idx].val);
  }
}
void lntable_clear(lntable* h)
{
  for(size_t idx=0;idx<h->capacity;idx++)
    h->table[idx].stat = LN_EMPTY;
}
void lntable_destroy(lntable* h)
{
  free(h->table);
}
