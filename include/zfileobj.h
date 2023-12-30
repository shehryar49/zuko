#ifndef ZFILEOBJECT_H
#define ZFILEOBJECT_H

#include <stdio.h>
#include <stdlib.h>

typedef struct zfile
{
  FILE* fp;
  bool open;
}zfile;

#endif