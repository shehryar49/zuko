#ifndef ZLIST_H
#define ZLIST_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zobject.h"

#ifdef __cplusplus
  extern "C"{
#endif
typedef struct zlist
{
  zobject* arr;
  size_t size;
  size_t capacity;
}zlist;

void zlist_init(zlist* p);
void zlist_push(zlist* p,zobject val);
void zlist_erase(zlist* p,size_t idx);
void zlist_erase_range(zlist* p,size_t i,size_t j);
void zlist_resize(zlist* p,size_t newSize);
bool zlist_pop(zlist* p,zobject* val);
void zlist_assign(zlist* p,zlist* val);
void zlist_insert(zlist* p,size_t idx,zobject val);
void zlist_insert_list(zlist* p,size_t idx,zlist* sublist);
void zlist_destroy(zlist* p);
void zlist_fastpop(zlist* p,zobject* val);

#ifdef __cplusplus
  }
#endif
#endif
