#ifndef PLUTONIUM_AST_H_
#define PLUTONIUM_AST_H_
#include <vector>
using namespace std;
enum NodeType
{
  declare,
  import,
  importas,
  assign,
  memb,
  WHILE,
  DOWHILE,
  FOR,
  FOREACH,
  NAMESPACE,
  IF,
  IFELSE,
  IFELIF,
  IFELIFELSE,
  THROW,
  TRYCATCH,
  FUNC,
  CORO,
  CLASS,
  EXTCLASS,
  RETURN,
  YIELD,
  BREAK,
  CONTINUE,
  call,
  gc,
  file,
  add,
  sub,
  mul,
  div,
  mod,
  xor,
  lshift,
  rshift,
  bitwiseand,
  bitwiseor,
  AND,
  OR,
  IS,
  lt,
  gt,
  lte,
  gte,
  neg,
  complement,
  not,
  index,
  dict,
  NUM,
  FLOAT,
};
struct Node
{
  string val;
  NodeType type;
  vector<Node>* childs;
};
#endif