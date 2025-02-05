#ifndef ZUKO_FUNOBJ_H_
#define ZUKO_FUNOBJ_H_
//#include "zapi.h"
#include "klass.h"
#include "zlist.h"

// struct to represent zuko code functions
typedef struct zfun
{
    /*
     * functions can be binded to classes as methods
     */
    zclass* _klass;
    const char* name;
    size_t i;
    size_t args;
    /* list of default/optional parameters */
    zlist opt;
}zfun;

#endif
