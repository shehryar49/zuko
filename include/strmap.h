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


typedef struct strmap {
  SM_Slot *table;
  size_t size;
  size_t capacity;
#ifdef __cpluscplus
  strmap &operator=(const strmap &) = delete;
  strmap(const strmap &) = delete;
#endif
}strmap;

size_t hashDJB2(const char *, size_t);

void strmap_init(strmap *h);
void strmap_set(strmap *h, const char *key, zobject val);
void strmap_emplace(strmap *h, const char *key, zobject val);
bool strmap_get(strmap *h, const char *key, zobject *val);
zobject *strmap_getRef(strmap *h, const char *key);
bool strmap_erase(strmap *h, const char *key);
void strmap_assign(strmap *h, strmap *other); // makes deep copy
void strmap_destroy(strmap *h);

#ifdef __cplusplus
}
#endif

#endif
