#ifndef ZUKO_STR_H_
#define ZUKO_STR_H_
#include <string.h>

#ifdef __cpluscplus
extern "C"{
#endif

// Immutable zuko string implementation
typedef struct zstr
{
  char* val; //actually const char*
  //but we need to write some value into it after malloc()  so ...
  size_t len; //for quick len check
}zstr;

#ifdef __cpluscplus
}
#endif

#endif