#include "strmap.h"
#include "zobject.h"

void strmap_init(strmap *h) {
  h->table = (SM_Slot *)malloc(sizeof(SM_Slot) * 4);
  h->size = 0;
  h->capacity = 4;
  for (int i = 0; i < 4; i++)
    h->table[i].stat = SM_EMPTY;
}
void strmap_set(strmap *h, const char *key, zobject val) {
  size_t idx;
  idx = hashDJB2(key, h->capacity);
  int i = 1;
  bool reassigned = false;
  while (h->table[idx].stat == SM_OCCUPIED) {
    if (strcmp(h->table[idx].key, key) == 0) // same key, just reassign value
    {
			reassigned = true;
      break;
    }
    idx = (idx + i * i) & (h->capacity - 1);
    i++;
  }
  h->table[idx].key = key;
  h->table[idx].val = val;
  h->table[idx].stat = SM_OCCUPIED;
  if (!reassigned)
    (h->size)++;
  if (h->size == 0.75f * h->capacity) // rehash
  {
    SM_Slot *newArr = (SM_Slot *)malloc(sizeof(SM_Slot) * (h->capacity * 2));
    for (size_t i = 0; i < h->capacity * 2; i++)
      newArr[i].stat = SM_EMPTY;
    SM_Slot *old = h->table;
    size_t oldcap = h->capacity;
    h->size = 0;
    h->table = newArr;
    h->capacity = h->capacity * 2;
    for (size_t i = 0; i < oldcap; i++) {
      if (old[i].stat == SM_OCCUPIED)
        strmap_set(h, old[i].key, old[i].val);
    }
    free(old);
  }
}
void strmap_emplace(strmap *h, const char *key, zobject val) {
  size_t idx;
  idx = hashDJB2(key, h->capacity);
  int i = 1;
  while (h->table[idx].stat == SM_OCCUPIED) {
    if (strcmp(h->table[idx].key, key) == 0) // same key, don't reassign value
      return;
    idx = (idx + i * i) & (h->capacity - 1);
    i++;
  }
  h->table[idx].key = key;
  h->table[idx].val = val;
  h->table[idx].stat = SM_OCCUPIED;
  (h->size)++;
  if (h->size == 0.75f * h->capacity) // rehash
  {
    SM_Slot *newArr = (SM_Slot *)malloc(sizeof(SM_Slot) * (h->capacity * 2));
    for (size_t i = 0; i < h->capacity * 2; i++)
      newArr[i].stat = SM_EMPTY;
    SM_Slot *old = h->table;
    size_t oldcap = h->capacity;
    h->size = 0;
    h->table = newArr;
    h->capacity = h->capacity * 2;
    for (size_t i = 0; i < oldcap; i++) {
      if (old[i].stat == SM_OCCUPIED)
        strmap_set(h, old[i].key, old[i].val);
    }
    free(old);
  }
}
bool strmap_get(strmap *h, const char *key, zobject *val) {
  size_t idx = hashDJB2(key, h->capacity);
  int i = 1;
  while (h->table[idx].stat != SM_EMPTY) {
    if (strcmp(h->table[idx].key, key) == 0) {
      *val = h->table[idx].val;
      return true;
    }
    idx = (idx + i * i) & (h->capacity - 1);
    i++;
  }
  return false;
}
zobject* strmap_getRef(strmap *h, const char *key) 
{
  size_t idx = hashDJB2(key, h->capacity);
  int i = 1;
  while (h->table[idx].stat != SM_EMPTY) 
  {
    if (strcmp(h->table[idx].key, key) == 0) 
      return &(h->table[idx].val);
    idx = (idx + i * i) & (h->capacity - 1);
    i++;
  }
  return NULL;
}
bool strmap_erase(strmap *h, const char *key) 
{
  size_t idx = hashDJB2(key, h->capacity);
  int i = 1;
  while (h->table[idx].stat != SM_EMPTY) 
  {
    if (strcmp(h->table[idx].key, key) == 0)
    {
      h->table[idx].stat = SM_DELETED;
      h->table[idx].key = NULL;
      h->table[idx].val.type = Z_NIL;
      h->size--;
      return true;
    }
    idx = (idx + i * i) & (h->capacity - 1);
    i++;
  }
  return false;
}
void strmap_assign(strmap *h, strmap *other) // makes deep copy
{
  for (size_t idx = 0; idx < other->capacity; idx++) {
    if (other->table[idx].stat != SM_OCCUPIED)
      continue;
    strmap_emplace(h, other->table[idx].key, other->table[idx].val);
  }
}
void strmap_destroy(strmap *h) { free(h->table); }
