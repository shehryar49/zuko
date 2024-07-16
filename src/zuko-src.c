#include "zuko-src.h"
#include "lntable.h"
#include "refgraph.h"

void zuko_src_init(zuko_src* src)
{
    src->num_of_constants = 0;
    str_vector_init(&src->files);
    str_vector_init(&src->sources);
    lntable_init(&src->line_num_table);
    refgraph_init(&src->ref_graph);
}

void zuko_src_add_file(zuko_src* src, char* filename, char* source)
{
    str_vector_push(&src->files,filename);
    str_vector_push(&src->sources,source);
}
void zuko_src_reset(zuko_src* src)
{
    src->num_of_constants = 0;
    src->files.size = 0;
    src->sources.size = 0;
    lntable_clear(&src->line_num_table);
    refgraph_clear(&src->ref_graph);
}