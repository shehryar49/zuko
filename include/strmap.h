#ifndef ZUKO_STRMAP_H_
#define ZUKO_STRMAP_H_
// Members of Zuko Classes, Objects and Modules
// are represented using this data structure.

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "zobject.h"
struct zobject;
typedef struct zobject zobject;

typedef enum SM_SlotStatus { SM_EMPTY, SM_OCCUPIED, SM_DELETED } SM_SlotStatus;
typedef struct SM_Slot {
  const char *key;
  zobject val;
  SM_SlotStatus stat;
} SM_Slot;


typedef struct StrMap {
  SM_Slot *table;
  size_t size;
  size_t capacity;
#ifdef __cpluscplus
  StrMap &operator=(const StrMap &) = delete;
  StrMap(const StrMap &) = delete;
#endif
} StrMap;

size_t hashDJB2(const char *, size_t);

void StrMap_init(StrMap *h);
void StrMap_set(StrMap *h, const char *key, zobject val);
void StrMap_emplace(StrMap *h, const char *key, zobject val);
bool StrMap_get(StrMap *h, const char *key, zobject *val);
zobject *StrMap_getRef(StrMap *h, const char *key);
bool StrMap_erase(StrMap *h, const char *key);
void StrMap_assign(StrMap *h, StrMap *other); // makes deep copy
void StrMap_destroy(StrMap *h);

#ifdef __cplusplus
}
#endif

#endif