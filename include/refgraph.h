#ifndef ZUKO_REFGRAPH_H_
#define ZUKO_REFGRAPH_H_

#ifdef __cplusplus
extern "C"{
#endif
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "str-vec.h"

typedef enum refgraph_slot_status
{
  REFGRAPH_SLOT_EMPTY,
  REFGRAPH_SLOT_OCCUPIED,
  REFGRAPH_SLOT_DELETED
}refgraph_slot_status;

typedef struct refgraph_slot
{
  char* key;
  str_vector val;
  refgraph_slot_status stat;
}refgraph_slot;

typedef struct refgraph
{
  refgraph_slot* table;
  size_t size;
  size_t capacity;
}refgraph;

size_t hashDJB2(const char* key,size_t M);


void refgraph_init(refgraph* h);
void refgraph_set(refgraph* h,char* key,str_vector val);
void refgraph_emplace(refgraph* h,char* key,str_vector val);
bool refgraph_get(refgraph* h,char* key,str_vector* val);
str_vector* refgraph_getref(refgraph* h,const char* key);
void refgraph_clear(refgraph* h);
void refgraph_destroy(refgraph* h);

#ifdef __cplusplus
}
#endif

#endif