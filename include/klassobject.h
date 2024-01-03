#ifndef ZUKO_KLASSOBJ_H_
#define ZUKO_KLASSOBJ_H_

#include "klass.h"
struct KlassObject
{
  Klass* klass;
  StrMap members;
  StrMap privateMembers;
};

#endif