#ifndef COROUTINEOBJ_H_
#define COROUTINEOBJ_H_
#include "funobject.h"

typedef enum coroutine_state
{
  COROUTINE_STATE_SUSPENDED,
  COROUTINE_STATE_RUNNING,
  COROUTINE_STATE_STOPPED,
}coroutine_state;

typedef struct coroutine
{
  int curr;//index in bytecode from where to resume the coroutine
  zlist locals;
  coroutine_state state;
  const char* name;
  zfun* fun;//function from which this coroutine object was made
  bool give_val_on_resume;
  #ifdef __cpluscplus
    coroutine(const coroutine&) = delete;
    coroutine& operator=(const coroutine&) = delete;
  #endif
}coroutine;

#endif
