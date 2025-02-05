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
#ifndef PARSER_H_
#define PARSER_H_
#include "refgraph.h"
#include "token.h"
#include "ast.h"
#include "zuko-src.h"
#include "lexer.h"
#include "str-vec.h"
#include "ptr-vector.h"

#ifdef __cplusplus
extern "C"{
#endif

Node* new_node(NodeType type,const char*);

int find_token(token t,int start,int end,token* tokens);
int find_token_consecutive(token t,int start,int end,token* tokens);

int match_token(int,int,token_type,token*);
int match_token_right(int,int,token_type,token*);

void delete_ast(Node* ast);
void copy_ast(Node** dest,Node* src);

// Function to print AST in tablular form
void print_ast(Node* n,int spaces);


typedef struct parser_ctx
{
  str_vector prefixes;//for namespaces 
  refgraph* refGraph;
  const char* currSym;
  str_vector* files;
  str_vector* sources;
  const char* filename;
  size_t line_num;
  bool foundYield ;
  //ptr_vector to_free;
  //Context
  bool infunc;
  bool inclass;
  bool inloop;
  bool inif;
  bool inelif;
  bool inelse;
  bool intry;
  bool incatch;
}parser_ctx;

char* merge_str(const char*,const char*);

parser_ctx* create_parser_context(zuko_src*);
void parser_init(parser_ctx*);
void parser_destroy(parser_ctx*);
void parser_set_source(parser_ctx*,zuko_src* p,size_t root_idx); // for error printing and stuff
Node* parse_block(parser_ctx*,token* tokens,int begin,int end);

#ifdef __cplusplus
}
#endif
#endif
