#ifndef TOKEN_H_
#define TOKEN_H_
#include <string>

using namespace std;
enum TokenType
{
  ID_TOKEN,
  STRING_TOKEN,
  NUM_TOKEN,
  FLOAT_TOKEN,
  KEYWORD_TOKEN,
  COMMA_TOKEN,
  COLON_TOKEN,
  L_CURLY_BRACKET_TOKEN,
  R_CURLY_BRACKET_TOKEN,
  LParen_TOKEN,
  RParen_TOKEN,
  BEGIN_LIST_TOKEN,
  END_LIST_TOKEN,
  OP_TOKEN,
  NEWLINE_TOKEN,
  BYTE_TOKEN,
  BOOL_TOKEN,
  EOP_TOKEN//end of program
};
struct Token
{
  string content;
  enum TokenType  type;
  size_t ln;
};
#endif
