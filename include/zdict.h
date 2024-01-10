#ifndef ZDICT_H_
#define ZDICT_H_

#include <stdlib.h>
#include <string.h>
#include "zapi.h"
#include "zstr.h"

typedef enum slotStatus
{
  EMPTY,
  OCCUPIED,
  DELETED
}slotStatus;
typedef struct slot
{
  ZObject key;
  ZObject val;
  slotStatus stat;
}slot;
typedef struct ZDict
{
  slot* table;
  size_t size;
  size_t capacity;
}ZDict;

size_t hashDJB2(const char* key,size_t M)
{
  //using djb2 hash function for strings
  size_t hash = 5381;
  unsigned char c;
  while((c = *key++))
  {
    //djb2 with XOR
    hash = ((hash << 5) + hash) ^ c; // hash*33 + c
  }
  //M is always a power of 2
  return hash & (M-1);
}

size_t hashZObject(ZObject a,size_t M)
{
  char t = a.type;
  if(t==Z_STR)
      return hashDJB2(((ZStr*)a.ptr) -> val,M);
  else if(t==Z_INT || t == Z_BOOL || t == Z_BYTE)
      return ((a.i * 31) & (M-1));
  else if(t==Z_INT64)
      return ((a.l * 31) & (M-1));
  else if(t==Z_FLOAT)
      return ((size_t)(a.f * 31) & (M-1));

  //other types not supported as keys in dictionaries
  return 0;
}
void ZDict_init(ZDict* h)
{
  h->table = (slot*)malloc(sizeof(slot)*4);
  h->size = 0;
  h->capacity = 4;
  for(int i=0;i<4;i++)
    h->table[i].stat = EMPTY;
}
void ZDict_set(ZDict* h,ZObject key,ZObject val)
{
  size_t idx = hashZObject(key,h->capacity);
  int i = 1;
  bool reassigned = false;
  while(h->table[idx].stat == OCCUPIED)
  {
    if(ZObject_equals(h->table[idx].key,key)) //same key, just reassign value
    {
      reassigned = true;
      break;
    }
    idx = (idx + i*i)  & (h->capacity - 1);
    i++;
  }
  h->table[idx].key = key;
  h->table[idx].val = val;
  h->table[idx].stat = OCCUPIED;
  if(!reassigned)
    (h->size)++;
  if(h->size  == 0.75f* h->capacity) //rehash
  {
    slot* newArr = (slot*)malloc(sizeof(slot)*(h->capacity*2));
    for(size_t i=0;i<h->capacity*2;i++)
      newArr[i].stat = EMPTY;
    slot* old = h->table;
    size_t oldcap = h->capacity;
    h->size = 0;
    h->table = newArr;
    h->capacity = h->capacity*2;
    for(size_t i=0;i<oldcap;i++)
    {
      if(old[i].stat == OCCUPIED)
        ZDict_set(h,old[i].key,old[i].val);
    }
    free(old);
  }
}
void ZDict_emplace(ZDict* h,ZObject key,ZObject val)
{
  size_t idx = hashZObject(key,h->capacity);
  int i = 1;
  while(h->table[idx].stat == OCCUPIED)
  {
    if(ZObject_equals(h->table[idx].key,key)) //same key, don't reassign value
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
    for(size_t i=0;i<h->capacity*2;i++)
      newArr[i].stat = EMPTY;
    slot* old = h->table;
    size_t oldcap = h->capacity;
    h->size = 0;
    h->table = newArr;
    h->capacity = h->capacity*2;
    for(size_t i=0;i<oldcap;i++)
    {
      if(old[i].stat == OCCUPIED)
        ZDict_set(h,old[i].key,old[i].val);
    }
    free(old);
  }
}
bool ZDict_get(ZDict* h,ZObject key,ZObject* val)
{
  size_t idx = hashZObject(key,h->capacity);
  int i = 1;
  while(h->table[idx].stat != EMPTY)
  {
    if(ZObject_equals(h->table[idx].key,key))
    {
      *val = h->table[idx].val;
      return true;
    }
    idx = (idx + i*i) & (h->capacity - 1);
    i++;
  }
  return false;
}
bool ZDict_erase(ZDict* h,ZObject key)
{
  size_t idx = hashZObject(key,h->capacity);
  int i = 1;
  while(h->table[idx].stat != EMPTY)
  {
    if(ZObject_equals(h->table[idx].key,key))
    {
      h->table[idx].stat = DELETED;
      h->table[idx].key.type = Z_NIL;
      h->table[idx].val.type = Z_NIL;
      h->size--;
      return true;
    }
    idx = (idx + i*i) & (h->capacity - 1);
    i++;
  }
  return false;
}
bool ZDict_equal(ZDict* h,ZDict* other)
{
  for(size_t idx = 0; idx < h->capacity;idx++)
  {
    if(h->table[idx].stat != OCCUPIED)
      continue;
    ZObject val;
    if(!ZDict_get(other,h->table[idx].key,&val) || !ZObject_equals(val,h->table[idx].val))
    {
      printf("mismatch at key %c %c\n",h->table[idx].val.type,val.type);
      return false;
    }
  }
  return true;
}
void ZDict_assign(ZDict* h,ZDict* other) // makes deep copy
{
  for(size_t idx=0;idx<other->capacity;idx++)
  {
    if(other->table[idx].stat != OCCUPIED)
      continue;
    ZDict_emplace(h,other->table[idx].key,other->table[idx].val);
  }
}
void ZDict_clear(ZDict* h)
{
  for(size_t idx=0;idx<h->capacity;idx++)
    h->table[idx].stat = EMPTY;
}
void ZDict_destroy(ZDict* h)
{
  free(h->table);
}
#endif
