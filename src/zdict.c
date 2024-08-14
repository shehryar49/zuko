#include "zdict.h"
#include "zstr.h"
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

size_t hashzobject(zobject a,size_t M)
{
  char t = a.type;
  if(t==Z_STR)
      return hashDJB2(((zstr*)a.ptr) -> val,M);
  else if(t==Z_INT || t == Z_BOOL || t == Z_BYTE)
      return ((a.i * 31) & (M-1));
  else if(t==Z_INT64)
      return ((a.l * 31) & (M-1));
  else if(t==Z_FLOAT)
      return ((size_t)(a.f * 31) & (M-1));

  //other types not supported as keys in dictionaries
  return 0;
}
void zdict_init(zdict* h)
{
  h->table = (slot*)malloc(sizeof(slot)*4);
  h->size = 0;
  h->capacity = 4;
  for(int i=0;i<4;i++)
    h->table[i].stat = EMPTY;
}
void zdict_set(zdict* h,zobject key,zobject val)
{
  size_t idx = hashzobject(key,h->capacity);
  int i = 1;
  bool reassigned = false;
  while(h->table[idx].stat == OCCUPIED)
  {
    if(zobject_equals(h->table[idx].key,key)) //same key, just reassign value
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
        zdict_set(h,old[i].key,old[i].val);
    }
    free(old);
  }
}
void zdict_emplace(zdict* h,zobject key,zobject val)
{
  size_t idx = hashzobject(key,h->capacity);
  int i = 1;
  while(h->table[idx].stat == OCCUPIED)
  {
    if(zobject_equals(h->table[idx].key,key)) //same key, don't reassign value
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
        zdict_set(h,old[i].key,old[i].val);
    }
    free(old);
  }
}
bool zdict_get(zdict* h,zobject key,zobject* val)
{
  size_t idx = hashzobject(key,h->capacity);
  int i = 1;
  while(h->table[idx].stat != EMPTY)
  {
    if(zobject_equals(h->table[idx].key,key))
    {
      *val = h->table[idx].val;
      return true;
    }
    idx = (idx + i*i) & (h->capacity - 1);
    i++;
  }
  return false;
}
bool zdict_erase(zdict* h,zobject key)
{
  size_t idx = hashzobject(key,h->capacity);
  int i = 1;
  while(h->table[idx].stat != EMPTY)
  {
    if(zobject_equals(h->table[idx].key,key))
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
bool zdict_equal(zdict* h,zdict* other)
{
  for(size_t idx = 0; idx < h->capacity;idx++)
  {
    if(h->table[idx].stat != OCCUPIED)
      continue;
    zobject val;
    if(!zdict_get(other,h->table[idx].key,&val) || !zobject_equals(val,h->table[idx].val))
    {
      return false;
    }
  }
  return true;
}
void zdict_assign(zdict* h,zdict* other) // makes deep copy
{
  for(size_t idx=0;idx<other->capacity;idx++)
  {
    if(other->table[idx].stat != OCCUPIED)
      continue;
    zdict_emplace(h,other->table[idx].key,other->table[idx].val);
  }
}
void zdict_clear(zdict* h)
{
  for(size_t idx=0;idx<h->capacity;idx++)
    h->table[idx].stat = EMPTY;
}
void zdict_destroy(zdict* h)
{
  free(h->table);
}
