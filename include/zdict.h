#ifndef ZDICT_H_
#define ZDICT_H_
#ifdef __cplusplus
extern "C"{
#endif
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "zobject.h"

typedef enum slotStatus
{
  EMPTY,
  OCCUPIED,
  DELETED
}slotStatus;
typedef struct slot
{
  zobject key;
  zobject val;
  slotStatus stat;
}slot;
typedef struct zdict
{
  slot* table;
  size_t size;
  size_t capacity;
}zdict;

size_t hashDJB2(const char* key,size_t M);

size_t hashzobject(zobject a,size_t M);

void zdict_init(zdict* h);
void zdict_set(zdict* h,zobject key,zobject val);
void zdict_emplace(zdict* h,zobject key,zobject val);
bool zdict_get(zdict* h,zobject key,zobject* val);
bool zdict_erase(zdict* h,zobject key);
bool zdict_equal(zdict* h,zdict* other);
void zdict_assign(zdict* h,zdict* other); // makes deep copy
void zdict_clear(zdict* h);
void zdict_destroy(zdict* h);

#ifdef __cplusplus
}
#endif
#endif
