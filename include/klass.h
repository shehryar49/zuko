#ifndef ZUKO_KLASS_H_
#define ZUKO_KLASS_H_

#include "strmap.h"

struct Klass
{
  const char* name;
  StrMap members;
  StrMap privateMembers;
};
#endif