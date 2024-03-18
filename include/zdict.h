#ifndef ZDICT_H_
#define ZDICT_H_
#ifdef __cplusplus
extern "C"{
#endif
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "zobject.h"
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

size_t hashDJB2(const char* key,size_t M);

size_t hashZObject(ZObject a,size_t M);

void ZDict_init(ZDict* h);
void ZDict_set(ZDict* h,ZObject key,ZObject val);
void ZDict_emplace(ZDict* h,ZObject key,ZObject val);
bool ZDict_get(ZDict* h,ZObject key,ZObject* val);
bool ZDict_erase(ZDict* h,ZObject key);
bool ZDict_equal(ZDict* h,ZDict* other);

void ZDict_assign(ZDict* h,ZDict* other); // makes deep copy
void ZDict_clear(ZDict* h);
void ZDict_destroy(ZDict* h);

#ifdef __cplusplus
}
#endif
#endif
