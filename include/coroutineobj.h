#ifndef COROUTINEOBJ_H_
#define COROUTINEOBJ_H_
#include "funobject.h"
typedef enum CoState
{
  SUSPENDED,
  RUNNING,
  STOPPED,
}CoState;
typedef struct Coroutine
{
  int curr;//index in bytecode from where to resume the coroutine
  ZList locals;
  CoState state;
  const char* name;
  FunObject* fun;//function from which this coroutine object was made
  bool giveValOnResume;
  #ifdef __cpluscplus
    Coroutine(const Coroutine&) = delete;
    Coroutine& operator=(const Coroutine&) = delete;
  #endif
}Coroutine;

#endif