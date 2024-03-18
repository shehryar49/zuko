#ifndef ZLIST_H
#define ZLIST_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zobject.h"

#ifdef __cplusplus
  extern "C"{
#endif
typedef struct ZList
{
  ZObject* arr;
  size_t size;
  size_t capacity;
}ZList;

void ZList_init(ZList* p);
void ZList_push(ZList* p,ZObject val);
void ZList_erase(ZList* p,size_t idx);
void ZList_eraseRange(ZList* p,size_t i,size_t j);
void ZList_resize(ZList* p,size_t newSize);
bool ZList_pop(ZList* p,ZObject* val);
void ZList_fastpop(ZList* p,ZObject* val);
void ZList_assign(ZList* p,ZList* val);
void ZList_insert(ZList* p,size_t idx,ZObject val);
void ZList_insertList(ZList* p,size_t idx,ZList* sublist);
void ZList_destroy(ZList* p);

#ifdef __cplusplus
  }
#endif
#endif