#ifndef ZUKO_BUILTINS_HASHMAP
#define ZUKO_BUILTINS_HASHMAP


#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "zobject.h"

typedef zobject(*BuiltinFunc)(zobject*,int32_t);

typedef enum bslot_status { BSLOT_EMPTY, BSLOT_OCCUPIED, BSLOT_DELETED } bslot_status;

typedef struct bslot 
{
    const char *key;
    BuiltinFunc val;
    bslot_status stat;
}bslot;


typedef struct bmap 
{
    bslot *table;
    size_t size;
    size_t capacity;
} bmap;

size_t hashDJB2(const char *, size_t);

void bmap_init(bmap *h);
void bmap_set(bmap *h, const char *key, BuiltinFunc val);
void bmap_emplace(bmap *h, const char *key, BuiltinFunc val);
bool bmap_get(bmap *h, const char *key, BuiltinFunc *val);
BuiltinFunc* bmap_getref(bmap *h, const char *key);
bool bmap_erase(bmap *h, const char *key);
void bmap_assign(bmap *h, bmap *other); // makes deep copy
void bmap_destroy(bmap *h);

#ifdef __cplusplus
}
#endif

#endif
