#ifndef ZUKO_SRC_H_
#define ZUKO_SRC_H_
#ifdef __cplusplus
extern "C"{
#endif

#include <stdlib.h>
#include "byte_src.h"
#include "str-vec.h"
#include "lntable.h"
#include "refgraph.h"

typedef struct zuko_src
{
    str_vector files;
    str_vector sources;
    lntable line_num_table;
    refgraph ref_graph; // generated by the parser and used by the compiler to eliminate dead functions and classes
}zuko_src;

zuko_src* create_source(const char* filename, char* src);
void zuko_src_init(zuko_src* src);
void zuko_src_add_file(zuko_src*,char*,char*);
void zuko_src_reset(zuko_src*);
void zuko_src_destroy(zuko_src* src);

#ifdef __cplusplus
}
#endif
#endif
