#ifndef COROUTINEOBJ_H_
#define COROUTINEOBJ_H_
#include "zapi.h"

enum CoState
{
  SUSPENDED,
  RUNNING,
  STOPPED,
};
struct Coroutine
{
  int curr;//index in bytecode from where to resume the coroutine
  zlist locals;
  CoState state;
  const char* name;
  FunObject* fun;//function from which this coroutine object was made
  bool giveValOnResume;
  Coroutine& operator=(const Coroutine&) = delete;
};

#endif