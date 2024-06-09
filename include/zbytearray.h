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

typedef struct zbyteArr
{
  uint8_t* arr;
  size_t size;
  size_t capacity;
}zbytearr;

void zbytearr_init(zbytearr* p);
void zbytearr_push(zbytearr* p,uint8_t val);
void zbytearr_erase(zbytearr* p,size_t idx);
void zbytearr_erase_range(zbytearr* p,size_t i,size_t j);
void zbytearr_resize(zbytearr* p,size_t newSize);
bool zbytearr_pop(zbytearr* p,uint8_t* val);
void zbytearr_fastpop(zbytearr* p,uint8_t* val);
void zbytearr_assign(zbytearr* p,zbytearr* val);
void zbytearr_insert(zbytearr* p,size_t idx,uint8_t val);
void zbytearr_insert_arr(zbytearr* p,size_t idx,zbytearr* sublist);
bool zbytearr_equal(zbytearr* a,zbytearr* b);
void zbytearr_destroy(zbytearr* p);
#ifdef __cplusplus
}
#endif

#endif