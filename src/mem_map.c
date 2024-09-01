#include "mem_map.h"

size_t hash_key(size_t a, size_t M)
{
    return ((a * 31) & (M-1));
}


void mem_map_init(mem_map* h)
{
  h->table = (memmap_slot*)malloc(sizeof(memmap_slot)*4);
  h->size = 0;
  h->capacity = 4;
  for(int i=0;i<4;i++)
    h->table[i].stat = MM_EMPTY;
}
void mem_map_set(mem_map* h,void* key,mem_info val)
{
  size_t idx = hash_key((size_t)key,h->capacity);
  int i = 1;
  bool reassigned = false;
  while(h->table[idx].stat == MM_OCCUPIED)
  {
    if(key == h->table[idx].key) //same key, just reassign value
    {
      reassigned = true;
      break;
    }
    idx = (idx + i*i)  & (h->capacity - 1);
    i++;
  }
  h->table[idx].key = key;
  h->table[idx].val = val;
  h->table[idx].stat = MM_OCCUPIED;
  if(!reassigned)
    (h->size)++;
  if(h->size  == 0.75f* h->capacity) //rehash
  {
    memmap_slot* newArr = (memmap_slot*)malloc(sizeof(memmap_slot)*(h->capacity*2));
    for(size_t i=0;i<h->capacity*2;i++)
      newArr[i].stat = MM_EMPTY;
    memmap_slot* old = h->table;
    size_t oldcap = h->capacity;
    h->size = 0;
    h->table = newArr;
    h->capacity = h->capacity*2;
    for(size_t i=0;i<oldcap;i++)
    {
      if(old[i].stat == MM_OCCUPIED)
        mem_map_set(h,old[i].key,old[i].val);
    }
    free(old);
  }
}
void mem_map_emplace(mem_map* h,void* key,mem_info val)
{
  size_t idx = hash_key((size_t)key,h->capacity);
  int i = 1;
  while(h->table[idx].stat == MM_OCCUPIED)
  {
    if(h->table[idx].key == key) //same key, don't reassign value
      return;
    idx = (idx + i*i)  & (h->capacity - 1);
    i++;
  }
  h->table[idx].key = key;
  h->table[idx].val = val;
  h->table[idx].stat = MM_OCCUPIED;
  (h->size)++;
  if(h->size  == 0.75f* h->capacity) //rehash
  {
    memmap_slot* newArr = (memmap_slot*)malloc(sizeof(memmap_slot)*(h->capacity*2));
    for(size_t i=0;i<h->capacity*2;i++)
      newArr[i].stat = MM_EMPTY;
    memmap_slot* old = h->table;
    size_t oldcap = h->capacity;
    h->size = 0;
    h->table = newArr;
    h->capacity = h->capacity*2;
    for(size_t i=0;i<oldcap;i++)
    {
      if(old[i].stat == MM_OCCUPIED)
        mem_map_set(h,old[i].key,old[i].val);
    }
    free(old);
  }
}
bool mem_map_get(mem_map* h,void* key,mem_info* val)
{
  size_t idx = hash_key((size_t)key,h->capacity);
  int i = 1;
  while(h->table[idx].stat != MM_EMPTY)
  {
    if(h->table[idx].stat == MM_OCCUPIED && key == h->table[idx].key)
    {
      *val = h->table[idx].val;
      return true;
    }
    idx = (idx + i*i) & (h->capacity - 1);
    i++;
  }
  return false;
}
mem_info* mem_map_getref(mem_map* h,void* key)
{
  size_t idx = hash_key((size_t)key,h->capacity);
  size_t i = 1;
  while(h->table[idx].stat != MM_EMPTY)
  {
    //printf("in a cycle, i = %zu, stat = %d, empty = %d\n",idx,h->table[idx].stat,MM_EMPTY);
    if(h->table[idx].stat == MM_OCCUPIED && key == h->table[idx].key)
    {
      return &(h->table[idx].val);
    }
    idx = (idx + i*i) & (h->capacity - 1);
    i++;
  }
  return NULL;
}

bool mem_map_erase(mem_map* h,void* key)
{
  size_t idx = hash_key((size_t)key,h->capacity);
  size_t i = 1;
  while(h->table[idx].stat != MM_EMPTY)
  {
    if(h->table[idx].key == key)
    {
      h->table[idx].stat = MM_DELETED;
      h->table[idx].key = 0;
      h->size--;
      return true;
    }
    idx = (idx + i*i) & (h->capacity - 1);
    i++;
  }
  return false;
}
void mem_map_clear(mem_map* h)
{
  for(size_t idx=0;idx<h->capacity;idx++)
    h->table[idx].stat = MM_EMPTY;
}
void mem_map_destroy(mem_map* h)
{
  free(h->table);
}
