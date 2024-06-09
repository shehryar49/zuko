#ifndef ZUKO_FUNOBJ_H_
#define ZUKO_FUNOBJ_H_
//#include "zapi.h"
#include "klass.h"
#include "zlist.h"
// Struct to represent zuko code functions
typedef struct zfun
{
  zclass* _klass;//functions can be binded to classes as methods in which case they will have access to private members of that class
  //and also keep the class alive with them
  const char* name;
  size_t i;
  size_t args;
  zlist opt; //default/optional parameters
}zfun;

#endif