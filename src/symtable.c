#include "symtable.h"
#include <stdio.h>
void symtable_init(symtable *h) {
  h->table = (symtable_slot *)malloc(sizeof(symtable_slot) * 4);
  h->size = 0;
  h->capacity = 4;
  for (int i = 0; i < 4; i++)
    h->table[i].stat = SYMT_EMPTY;
}
void symtable_set(symtable *h, const char *key, size_t val) {
  size_t idx;
  idx = hashDJB2(key, h->capacity);
  int i = 1;
  bool reassigned = false;
  while (h->table[idx].stat == SYMT_OCCUPIED) {
    if (h->table[idx].stat == SYMT_OCCUPIED && strcmp(h->table[idx].key, key) == 0) // same key, just reassign value
    {
			reassigned = true;
      break;
    }
    idx = (idx + i * i) & (h->capacity - 1);
    i++;
  }
  h->table[idx].key = key;
  h->table[idx].val = val;
  h->table[idx].stat = SYMT_OCCUPIED;
  if (!reassigned)
    (h->size)++;
  if (h->size == 0.75f * h->capacity) // rehash
  {
    symtable_slot *newArr = (symtable_slot *)malloc(sizeof(symtable_slot) * (h->capacity * 2));
    for (size_t i = 0; i < h->capacity * 2; i++)
      newArr[i].stat = SYMT_EMPTY;
    symtable_slot *old = h->table;
    size_t oldcap = h->capacity;
    h->size = 0;
    h->table = newArr;
    h->capacity = h->capacity * 2;
    for (size_t i = 0; i < oldcap; i++) {
      if (old[i].stat == SYMT_OCCUPIED)
        symtable_set(h, old[i].key, old[i].val);
    }
    free(old);
  }
}
void symtable_clear(symtable* h)
{
  for(size_t i=0;i<h->capacity; i++)
    h->table[i].stat = SYMT_EMPTY;
  h->size = 0;
}
void symtable_emplace(symtable *h, const char *key, size_t val) 
{
  size_t idx;
  idx = hashDJB2(key, h->capacity);
  int i = 1;
  while (h->table[idx].stat == SYMT_OCCUPIED) {
    if (h->table[idx].stat == SYMT_OCCUPIED && strcmp(h->table[idx].key, key) == 0) // same key, don't reassign value
    {
      return;
    }
    idx = (idx + i * i) & (h->capacity - 1);
    i++;
  }
  h->table[idx].key = key;
  h->table[idx].val = val;
  h->table[idx].stat = SYMT_OCCUPIED;
  (h->size)++;
  if (h->size == 0.75f * h->capacity) // rehash
  {
    symtable_slot *newArr = (symtable_slot *)malloc(sizeof(symtable_slot) * (h->capacity * 2));
    for (size_t i = 0; i < h->capacity * 2; i++)
      newArr[i].stat = SYMT_EMPTY;
    symtable_slot *old = h->table;
    size_t oldcap = h->capacity;
    h->size = 0;
    h->table = newArr;
    h->capacity = h->capacity * 2;
    for (size_t i = 0; i < oldcap; i++) {
      if (old[i].stat == SYMT_OCCUPIED)
        symtable_set(h, old[i].key, old[i].val);
    }
    free(old);
  }
}
bool symtable_get(symtable *h, const char *key, size_t *val)
{
  size_t idx = hashDJB2(key, h->capacity);
  int i = 1;
  while (h->table[idx].stat != SYMT_EMPTY) {
    if (h->table[idx].stat == SYMT_OCCUPIED && strcmp(h->table[idx].key, key) == 0) {
      *val = h->table[idx].val;
      return true;
    }
    idx = (idx + i * i) & (h->capacity - 1);
    i++;
  }
  return false;
}
size_t* symtable_getRef(symtable *h, const char *key) 
{
  size_t idx = hashDJB2(key, h->capacity);
  int i = 1;
  while (h->table[idx].stat != SYMT_EMPTY) 
  {
    if (h->table[idx].stat == SYMT_OCCUPIED && strcmp(h->table[idx].key, key) == 0) 
      return &(h->table[idx].val);
    idx = (idx + i * i) & (h->capacity - 1);
    i++;
  }
  return NULL;
}
bool symtable_erase(symtable *h, const char *key) 
{
  size_t idx = hashDJB2(key, h->capacity);
  int i = 1;
  while (h->table[idx].stat != SYMT_EMPTY) 
  {
    if (h->table[idx].stat == SYMT_OCCUPIED && strcmp(h->table[idx].key, key) == 0)
    {
      h->table[idx].stat = SYMT_DELETED;
      h->table[idx].key = NULL;
      h->size--;
      return true;
    }
    idx = (idx + i * i) & (h->capacity - 1);
    i++;
  }
  return false;
}
void symtable_assign(symtable *h, symtable *other) // makes deep copy
{
  for (size_t idx = 0; idx < other->capacity; idx++) {
    if (other->table[idx].stat != SYMT_OCCUPIED)
      continue;
    symtable_emplace(h, other->table[idx].key, other->table[idx].val);
  }
}
void symtable_destroy(symtable *h) { free(h->table); }