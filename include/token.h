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
  Token()
  {}
  Token(const Token& obj)
  {
    content = obj.content;
    type = obj.type;
    ln = obj.ln;
  }
  Token(TokenType type,const string& content,size_t line)
  {
    this->type = type;
    this->content = content;
    this->ln = line;
  }
  Token(TokenType type,char ch,size_t line)
  {
    this->type = type;
    this->content += ch;
    this->ln = line;
  }
  
  Token& operator=(const Token& obj)
  {
    if(&obj == this)
      return *this;
    content = obj.content;
    type = obj.type;
    ln = obj.ln;
    return *this;
  }
};
#endif
