#include "builtin-map.h"

void bmap_init(bmap *h) {
  h->table = (bslot *)malloc(sizeof(bslot) * 4);
  h->size = 0;
  h->capacity = 4;
  for (int i = 0; i < 4; i++)
    h->table[i].stat = BSLOT_EMPTY;
}
void bmap_set(bmap *h, const char *key, BuiltinFunc val) {
  size_t idx;
  idx = hashDJB2(key, h->capacity);
  int i = 1;
  bool reassigned = false;
  while (h->table[idx].stat == BSLOT_OCCUPIED) {
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
  h->table[idx].stat = BSLOT_OCCUPIED;
  if (!reassigned)
    (h->size)++;
  if (h->size == 0.75f * h->capacity) // rehash
  {
    bslot *newArr = (bslot *)malloc(sizeof(bslot) * (h->capacity * 2));
    for (size_t i = 0; i < h->capacity * 2; i++)
      newArr[i].stat = BSLOT_EMPTY;
    bslot *old = h->table;
    size_t oldcap = h->capacity;
    h->size = 0;
    h->table = newArr;
    h->capacity = h->capacity * 2;
    for (size_t i = 0; i < oldcap; i++) {
      if (old[i].stat == BSLOT_OCCUPIED)
        bmap_set(h, old[i].key, old[i].val);
    }
    free(old);
  }
}
void bmap_emplace(bmap *h, const char *key, BuiltinFunc val) {
  size_t idx;
  idx = hashDJB2(key, h->capacity);
  int i = 1;
  while (h->table[idx].stat == BSLOT_OCCUPIED) {
    if (strcmp(h->table[idx].key, key) == 0) // same key, don't reassign value
      return;
    idx = (idx + i * i) & (h->capacity - 1);
    i++;
  }
  h->table[idx].key = key;
  h->table[idx].val = val;
  h->table[idx].stat = BSLOT_OCCUPIED;
  (h->size)++;
  if (h->size == 0.75f * h->capacity) // rehash
  {
    bslot *newArr = (bslot *)malloc(sizeof(bslot) * (h->capacity * 2));
    for (size_t i = 0; i < h->capacity * 2; i++)
      newArr[i].stat = BSLOT_EMPTY;
    bslot *old = h->table;
    size_t oldcap = h->capacity;
    h->size = 0;
    h->table = newArr;
    h->capacity = h->capacity * 2;
    for (size_t i = 0; i < oldcap; i++) {
      if (old[i].stat == BSLOT_OCCUPIED)
        bmap_set(h, old[i].key, old[i].val);
    }
    free(old);
  }
}
bool bmap_get(bmap *h, const char *key, BuiltinFunc *val) {
  size_t idx = hashDJB2(key, h->capacity);
  int i = 1;
  while (h->table[idx].stat != BSLOT_EMPTY) {
    if (strcmp(h->table[idx].key, key) == 0) {
      *val = h->table[idx].val;
      return true;
    }
    idx = (idx + i * i) & (h->capacity - 1);
    i++;
  }
  return false;
}
BuiltinFunc* bmap_getRef(bmap *h, const char *key) 
{
  size_t idx = hashDJB2(key, h->capacity);
  int i = 1;
  while (h->table[idx].stat != BSLOT_EMPTY) 
  {
    if (strcmp(h->table[idx].key, key) == 0) 
      return &(h->table[idx].val);
    idx = (idx + i * i) & (h->capacity - 1);
    i++;
  }
  return NULL;
}
bool bmap_erase(bmap *h, const char *key) 
{
  size_t idx = hashDJB2(key, h->capacity);
  int i = 1;
  while (h->table[idx].stat != BSLOT_EMPTY) 
  {
    if (strcmp(h->table[idx].key, key) == 0)
    {
      h->table[idx].stat = BSLOT_DELETED;
      h->table[idx].key = NULL;
      h->table[idx].val = NULL;
      h->size--;
      return true;
    }
    idx = (idx + i * i) & (h->capacity - 1);
    i++;
  }
  return false;
}
void bmap_assign(bmap *h, bmap *other) // makes deep copy
{
  for (size_t idx = 0; idx < other->capacity; idx++) {
    if (other->table[idx].stat != BSLOT_OCCUPIED)
      continue;
    bmap_emplace(h, other->table[idx].key, other->table[idx].val);
  }
}
void bmap_destroy(bmap *h) { free(h->table); }