#ifndef ZBYTEARRAY_H
#define ZBYTEARRAY_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C"{
#endif

typedef struct ZByteArr
{
  uint8_t* arr;
  size_t size;
  size_t capacity;
}ZByteArr;

void ZByteArr_init(ZByteArr* p);
void ZByteArr_push(ZByteArr* p,uint8_t val);
void ZByteArr_erase(ZByteArr* p,size_t idx);
void ZByteArr_eraseRange(ZByteArr* p,size_t i,size_t j);
void ZByteArr_resize(ZByteArr* p,size_t newSize);
bool ZByteArr_pop(ZByteArr* p,uint8_t* val);
void ZByteArr_fastpop(ZByteArr* p,uint8_t* val);
void ZByteArr_assign(ZByteArr* p,ZByteArr* val);
void ZByteArr_insert(ZByteArr* p,size_t idx,uint8_t val);
void ZByteArr_insertArr(ZByteArr* p,size_t idx,ZByteArr* sublist);
bool ZByteArr_equal(ZByteArr* a,ZByteArr* b);
void ZByteArr_destroy(ZByteArr* p);
#ifdef __cplusplus
}
#endif

#endif