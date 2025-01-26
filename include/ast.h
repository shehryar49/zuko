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
  declare,
  import,
  importas,
  assign,
  memb,
  WHILE,
  DOWHILE,
  FOR,
  DFOR,//down to for loop
  decl_node,//used with FOR loop to tell if loop control variable should be
  //declared or not
  //i.e for(var i=0 to 10 step 1)
  nodecl,
  FOREACH,
  NAMESPACE,
  IF,
  IFELSE,
  IFELIF,
  IFELIFELSE,
  THROW_node,
  TRYCATCH,
  FUNC,
  CORO,
  CLASS,
  EXTCLASS,
  RETURN_NODE,//RETURN name collides with some name in libreadline
  EOP, //end of program, also marks the end of tree
  YIELD_node,
  BREAK_node,
  CONTINUE_node,
  call,
  args_node,
  line_node,
  gc,
  file_node,
  end,
  add,
  sub,
  mul,
  div_node,
  mod,
  XOR_node,
  lshift,
  rshift,
  bitwiseand,
  bitwiseor,
  AND,
  OR,
  IS_node,
  lt,
  gt,
  lte,
  gte,
  equal,
  noteq,
  neg,
  complement,
  NOT_node,
  index_node,
  dict,
  list,
  NUM,
  FLOAT,
  NIL,
  STR_NODE,
  BOOL_NODE,
  BYTE_NODE,
  ID_NODE,
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
