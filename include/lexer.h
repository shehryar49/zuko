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
#ifndef LEXER_H_
#define LEXER_H_

#include <vector>
#include "zuko-src.h"
#include "token.h"

typedef struct lexer
{
  const char* filename;
  const char* source_code;
  size_t srcLen;
  bool printErr = true;
  size_t line_num;  
  bool hadErr = false;
  const char* errmsg = nullptr;
  size_t k;
  char buffer[200];
}lexer;

bool isKeyword(const char* s);
token resolveMacro(const char* name);
void str(lexer* ctx,std::vector<token>&);
void macro(lexer* ctx,std::vector<token>&);
void comment(lexer* ctx,std::vector<token>&);
void numeric(lexer* ctx,std::vector<token>&);
void id(lexer* ctx,std::vector<token>&);

void lexErr(lexer* ctx,const char* type,const char* msg);
std::vector<token> lexer_generateTokens(lexer* ctx,const zuko_src* src,bool printErr = true,size_t root_idx = 0);

#endif // LEXER_H_
