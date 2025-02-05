/*MIT License

Copyright (c) 2022 Shahryar Ahmad 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/
#ifndef ZUKO_AST_H_
#define ZUKO_AST_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "nodeptr_vec.h"

typedef enum NodeType
{
  declare_node,
  import_node,
  importas_node,
  assign_node,
  memb_node,
  while_node,
  dowhile_node,
  for_node,
  dfor_node,//down to for loop
  decl_node,//used with FOR loop to tell if loop control variable should be
  //declared or not
  //i.e for(var i=0 to 10 step 1)
  nodecl_node,
  foreach_node,
  namespace_node,
  if_node,
  ifelse_node,
  ifelif_node,
  ifelifelse_node,
  throw_node,
  trycatch_node,
  func_node,
  coro_node,
  class_node,
  extclass_node,
  return_node,//RETURN name collides with some name in libreadline
  eop_node, //end of program, also marks the end of tree
  yield_node,
  break_node,
  continue_node,
  call_node,
  args_node,
  line_node,
  gc_node,
  file_node,
  end_node,
  add_node,
  sub_node,
  mul_node,
  div_node,
  mod_node,
  xor_node,
  lshift_node,
  rshift_node,
  bitwiseand_node,
  bitwiseor_node,
  and_node,
  or_node,
  is_node,
  lt_node,
  gt_node,
  lte_node,
  gte_node,
  equal_node,
  noteq_node,
  neg_node,
  complement_node,
  not_node,
  index_node,
  dict_node,
  list_node,
  num_node,
  float_node,
  nil_node,
  str_node,
  bool_node,
  byte_node,
  id_node,
  conditions_node,
}NodeType;
// No oop nonsense
typedef struct Node
{
  char* val;
  NodeType type;
  nodeptr_vector childs;
}Node;


extern const char* NodeTypeNames[68];

#ifdef __cplusplus
}
#endif

#endif
