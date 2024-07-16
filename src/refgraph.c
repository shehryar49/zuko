#include "refgraph.h"


void refgraph_init(refgraph* h)
{
  h->table = (refgraph_slot*)malloc(sizeof(refgraph_slot)*4);
  h->size = 0;
  h->capacity = 4;
  for(int i=0;i<4;i++)
    h->table[i].stat = REFGRAPH_SLOT_EMPTY;
}
void refgraph_set(refgraph* h,char* key,str_vector val)
{
  size_t idx = hashDJB2(key,h->capacity);
  int i = 1;
  bool reassigned = false;
  while(h->table[idx].stat == REFGRAPH_SLOT_OCCUPIED)
  {
    if(strcmp(h->table[idx].key,key) == 0) //same key, just reassign value
    {
      reassigned = true;
      break;
    }
    idx = (idx + i*i)  & (h->capacity - 1);
    i++;
  }
  h->table[idx].key = key;
  h->table[idx].val = val;
  h->table[idx].stat = REFGRAPH_SLOT_OCCUPIED;
  if(!reassigned)
    (h->size)++;
  if(h->size  == 0.75f* h->capacity) //rehash
  {
    refgraph_slot* newArr = (refgraph_slot*)malloc(sizeof(refgraph_slot)*(h->capacity*2));
    for(size_t i=0;i<h->capacity*2;i++)
      newArr[i].stat = REFGRAPH_SLOT_EMPTY;
    refgraph_slot* old = h->table;
    size_t oldcap = h->capacity;
    h->size = 0;
    h->table = newArr;
    h->capacity = h->capacity*2;
    for(size_t i=0;i<oldcap;i++)
    {
      if(old[i].stat == REFGRAPH_SLOT_OCCUPIED)
        refgraph_set(h,old[i].key,old[i].val);
    }
    free(old);
  }
}
void refgraph_emplace(refgraph* h,char* key,str_vector val)
{
  size_t idx = hashDJB2(key,h->capacity);
  int i = 1;
  while(h->table[idx].stat == REFGRAPH_SLOT_OCCUPIED)
  {
    if(strcmp(h->table[idx].key,key) == 0) //same key, don't reassign value
      return;
    idx = (idx + i*i)  & (h->capacity - 1);
    i++;
  }
  h->table[idx].key = key;
  h->table[idx].val = val;
  h->table[idx].stat = REFGRAPH_SLOT_OCCUPIED;
  (h->size)++;
  if(h->size  == 0.75f* h->capacity) //rehash
  {
    refgraph_slot* newArr = (refgraph_slot*)malloc(sizeof(refgraph_slot)*(h->capacity*2));
    for(size_t i=0;i<h->capacity*2;i++)
      newArr[i].stat = REFGRAPH_SLOT_EMPTY;
    refgraph_slot* old = h->table;
    size_t oldcap = h->capacity;
    h->size = 0;
    h->table = newArr;
    h->capacity = h->capacity*2;
    for(size_t i=0;i<oldcap;i++)
    {
      if(old[i].stat == REFGRAPH_SLOT_OCCUPIED)
        refgraph_set(h,old[i].key,old[i].val);
    }
    free(old);
  }
}
bool refgraph_get(refgraph* h,char* key,str_vector* val)
{
  size_t idx = hashDJB2(key,h->capacity);
  int i = 1;
  while(h->table[idx].stat != REFGRAPH_SLOT_EMPTY)
  {
    if(strcmp(h->table[idx].key,key) == 0)
    {
      *val = h->table[idx].val;
      return true;
    }
    idx = (idx + i*i) & (h->capacity - 1);
    i++;
  }
  return false;
}
str_vector* refgraph_getref(refgraph* h,const char* key)
{
  size_t idx = hashDJB2(key,h->capacity);
  int i = 1;
  while(h->table[idx].stat != REFGRAPH_SLOT_EMPTY)
  {
    if(strcmp(h->table[idx].key,key) == 0)
    {
      return &(h->table[idx].val);
    }
    idx = (idx + i*i) & (h->capacity - 1);
    i++;
  }
  return NULL;
}
void refgraph_clear(refgraph* h)
{
  for(size_t idx=0;idx<h->capacity;idx++)
    h->table[idx].stat = REFGRAPH_SLOT_EMPTY;
}
void refgraph_destroy(refgraph* h)
{
  free(h->table);
}