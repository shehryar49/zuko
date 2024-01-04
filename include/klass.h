#ifndef ZUKO_KLASS_H_
#define ZUKO_KLASS_H_

#include "strmap.h"

typedef struct Klass
{
  const char* name;
  StrMap members;
  StrMap privateMembers;
}Klass;
#endif