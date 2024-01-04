#ifndef MODULE_H_
#define MODULE_H_
#include "zapi.h"
struct Module
{
  const char* name;
  StrMap members;
};

#endif