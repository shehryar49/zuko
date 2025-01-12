#ifndef _CONVO_SIMPLE_H_
#define _CONVO_SIMPLE_H_
//This file contains functions for performing various conversions

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "misc.h"

#ifdef __cplusplus
extern "C"{
#endif

char* int32_to_str(int32_t x);
char* int64_to_str(int64_t l);
char* double_to_str(double);

double str_to_double(const char* s);
int32_t str_to_int32(const char* s);
int64_t str_to_int64(const char* s);

bool is_int64(const char* s);
bool is_int32(const char* s);
bool isdouble(const char* s);

#ifdef __cplusplus
}
#endif


#endif
////
