#ifndef ZUKO_SIZET_VECTOR
#define ZUKO_SIZET_VECTOR
#ifdef __cplusplus
extern "C"{
#endif
#include <stdlib.h>
#include <stdbool.h>

typedef struct sizet_vector
{
  size_t* arr;
  size_t size;
  size_t capacity;
}sizet_vector;

void sizet_vector_init(sizet_vector* p);
void sizet_vector_push(sizet_vector* p,size_t val);
void sizet_vector_erase(sizet_vector* p,size_t idx);
void sizet_vector_erase_range(sizet_vector* p,size_t i,size_t j);
void sizet_vector_resize(sizet_vector* p,size_t newSize);
bool sizet_vector_pop(sizet_vector* p,size_t* val);

inline void sizet_vector_fastpop(sizet_vector* p,size_t* val)
{
  *val = p->arr[--p->size];
}
void sizet_vector_assign(sizet_vector* p,sizet_vector* val);
void sizet_vector_insert(sizet_vector* p,size_t idx,size_t val);
void sizet_vector_insert_list(sizet_vector* p,size_t idx,sizet_vector* sublist);
void sizet_vector_destroy(sizet_vector* p);

#ifdef __cplusplus
}
#endif
#endif