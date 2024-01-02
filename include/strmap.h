#ifndef ZUKO_STRMAP_H_
#define ZUKO_STRMAP_H_

#include <stdlib.h>
#include <string.h>
#include "zobject.h"

#ifdef __cpluscplus
extern "C"{
#endif

typedef enum slotStatus
{
  EMPTY,
  OCCUPIED,
  DELETED
}slotStatus;
typedef struct slot
{
  const char* key;
  ZObject val;
  slotStatus stat;
}slot;
typedef struct ZSMap
{
  slot* table;
  size_t size;
  size_t capacity;
}ZSMap;

size_t hashDJB2(const char* key,size_t M)
{
  //using djb2 hash function for strings
  size_t hash = 5381;
  unsigned char c;
  while(c = *key++)
  {
    //djb2 with XOR
    hash = ((hash << 5) + hash) ^ c; // hash*33 + c
  }
  //M is always a power of 2
  return hash & (M-1);
}

void ZSMap_init(ZSMap* h)
{
  h->table = (slot*)malloc(sizeof(slot)*4);
  h->size = 0;
  h->capacity = 4;
  for(int i=0;i<4;i++)
    h->table[i].stat = EMPTY;
}
void ZSMap_set(ZSMap* h,const char* key,ZObject val)
{
  size_t idx;
  idx = hashDJB2(key,h->capacity);
  int i = 1;
  size_t copy = idx;
  while(h->table[idx].stat == OCCUPIED)
  {
    if(strcmp(h->table[idx].key,key) == 0) //same key, just reassign value
      break;
    idx = (idx + i*i)  & (h->capacity - 1);
    i++;
  }
  h->table[idx].key = key;
  h->table[idx].val = val;
  h->table[idx].stat = OCCUPIED;
  (h->size)++;
  if(h->size  == 0.75f* h->capacity) //rehash
  {
    slot* newArr = (slot*)malloc(sizeof(slot)*(h->capacity*2));
    for(int i=0;i<h->capacity*2;i++)
      newArr[i].stat = EMPTY;
    slot* old = h->table;
    size_t oldcap = h->capacity;
    h->size = 0;
    h->table = newArr;
    h->capacity = h->capacity*2;
    for(int i=0;i<oldcap;i++)
    {
      if(old[i].stat == OCCUPIED)
        ZSMap_set(h,old[i].key,old[i].val);
    }
    free(old);
  }
}
void ZSMap_emplace(ZSMap* h,const char* key,ZObject val)
{
  size_t idx;
  idx = hashDJB2(key,h->capacity);
  int i = 1;
  size_t copy = idx;
  while(h->table[idx].stat == OCCUPIED)
  {
    if(strcmp(h->table[idx].key,key) == 0) //same key, don't reassign value
      return;
    idx = (idx + i*i)  & (h->capacity - 1);
    i++;
  }
  h->table[idx].key = key;
  h->table[idx].val = val;
  h->table[idx].stat = OCCUPIED;
  (h->size)++;
  if(h->size  == 0.75f* h->capacity) //rehash
  {
    slot* newArr = (slot*)malloc(sizeof(slot)*(h->capacity*2));
    for(int i=0;i<h->capacity*2;i++)
      newArr[i].stat = EMPTY;
    slot* old = h->table;
    size_t oldcap = h->capacity;
    h->size = 0;
    h->table = newArr;
    h->capacity = h->capacity*2;
    for(int i=0;i<oldcap;i++)
    {
      if(old[i].stat == OCCUPIED)
        ZSMap_set(h,old[i].key,old[i].val);
    }
    free(old);
  }
}
bool ZSMap_get(ZSMap* h,const char* key,ZObject* val)
{
  size_t idx = hashDJB2(key,h->capacity);
  int i = 1;
  while(h->table[idx].stat != EMPTY)
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
bool ZSMap_erase(ZSMap* h,const char* key)
{
  size_t idx = hashDJB2(key,h->capacity);
  int i = 1;
  while(h->table[idx].stat != EMPTY)
  {
    if(strcmp(h->table[idx].key,key) == 0)
    {
      h->table[idx].stat = DELETED;
      h->table[idx].key = NULL;
      h->table[idx].val.type = Z_NIL;
      h->size--;
      return true;
    }
    idx = (idx + i*i) & (h->capacity - 1);
    i++;
  }
  return false;
}
void ZSMap_destroy(ZSMap* h)
{
  free(h->table);
}

#ifdef __cpluscplus
}
#endif

#endif