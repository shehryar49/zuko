#ifndef ZUKO_BYTESRC_H_
#define ZUKO_BYTESRC_H_

#include <stdlib.h>

typedef struct byte_src
{
    short file_index; // index of filename in files array (stored in zuko-src)
    size_t ln; // the line number
}byte_src;

#endif
