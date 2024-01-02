#ifndef ZUKO_STR_H_
#define ZUKO_STR_H_
#include <string.h>

#ifdef __cpluscplus
extern "C"{
#endif

// Immuutable zuko string implementation
typedef struct ZStr
{
  char* val;
  size_t len; //for quick len check
}ZStr;

#ifdef __cpluscplus
}
#endif

#endif