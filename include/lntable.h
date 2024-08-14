#ifndef ZUKO_LNTABLE_H_
#define ZUKO_LNTABLE_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "byte_src.h"

typedef enum lntable_slot_status
{
  LN_EMPTY,
  LN_OCCUPIED,
  LN_DELETED
}lntable_slot_status;
typedef struct lntable_slot
{
  size_t key;
  byte_src val;
  lntable_slot_status stat;
}lntable_slot;
typedef struct lntable
{
  lntable_slot* table;
  size_t size;
  size_t capacity;
}lntable;




void lntable_init(lntable* h);
void lntable_set(lntable* h,size_t key,byte_src val);
void lntable_emplace(lntable* h,size_t key,byte_src val);
bool lntable_get(lntable* h,size_t key,byte_src* val);
bool lntable_erase(lntable* h,size_t key);
void lntable_assign(lntable* h,lntable* other); // makes deep copy
void lntable_clear(lntable* h);
void lntable_destroy(lntable* h);

#ifdef __cplusplus
}
#endif

#endif
