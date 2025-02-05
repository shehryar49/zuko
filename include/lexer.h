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


#include "zuko-src.h"
#include "token.h"
#include "token-vector.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif

char* int64_to_string(int64_t x);

typedef struct lexer_ctx
{
  const char* filename;
  const char* source_code;
  size_t srcLen;
  bool printErr;
  size_t line_num;  
  bool hadErr;
  const char* errmsg;
  size_t k;
  char buffer[200];
}lexer_ctx;

bool isKeyword(const char* s);
token resolve_macro(lexer_ctx* ctx,const char* name,size_t length);
void str(lexer_ctx* ctx,token_vector*);
void macro(lexer_ctx* ctx,token_vector*);
void comment(lexer_ctx* ctx,token_vector*);
void numeric(lexer_ctx* ctx,token_vector*);
void id(lexer_ctx* ctx,token_vector*);

void lexErr(lexer_ctx* ctx,const char* type,const char* msg);
token_vector tokenize(lexer_ctx* ctx,const zuko_src* src,bool printErr,size_t root_idx,size_t start);

#ifdef __cplusplus
}
#endif

#endif // LEXER_H_
