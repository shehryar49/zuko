#ifndef ZUKO_SYMTABLE_VECTOR_H_
#define ZUKO_SYMTABLE_VECTOR_H_


#ifdef __cplusplus
  extern "C"{
#endif
#include "symtable.h"

typedef struct symtable_vector
{
  symtable** arr;
  size_t size;
  size_t capacity;
}symtable_vector;

void symtable_vector_init(symtable_vector* p);
void symtable_vector_push(symtable_vector* p,symtable val);
void symtable_vector_erase(symtable_vector* p,size_t idx);
void symtable_vector_erase_range(symtable_vector* p,size_t i,size_t j);
void symtable_vector_resize(symtable_vector* p,size_t newSize);
bool symtable_vector_pop(symtable_vector* p,symtable* val);
inline void symtable_vector_fastpop(symtable_vector* p,symtable* val)
{
  *val = p->arr[--p->size];
}
void symtable_vector_destroy(symtable_vector* p);

#ifdef __cplusplus
  }
#endif

#endif
