#ifndef ZUKO_SYMTABLE_H_
#define ZUKO_SYMTABLE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


typedef enum symtable_slot_status { SYMT_EMPTY, SYMT_OCCUPIED, SYMT_DELETED } symtable_slot_status;

typedef struct symtable_slot
{
  const char *key;
  size_t val;
  symtable_slot_status stat;
}symtable_slot;


typedef struct symtable
{
  symtable_slot* table;
  size_t size;
  size_t capacity;
} symtable;

size_t hashDJB2(const char *, size_t);

void symtable_init(symtable *h);
void symtable_set(symtable *h, const char *key, size_t val);
void symtable_emplace(symtable *h, const char *key, size_t val);
bool symtable_get(symtable *h, const char *key, size_t* val);
size_t* symtable_getref(symtable *h, const char *key);
bool symtable_erase(symtable *h, const char *key);
void symtable_assign(symtable *h, symtable *other); // makes deep copy
void symtable_clear(symtable* h);
void symtable_destroy(symtable *h);

#ifdef __cplusplus
}
#endif
#endif
