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
#include "utility.h"
#include "token.h"
#include "ast.h"
#include "zuko-src.h"
#include "lexer.h"
#include <algorithm>
#include <queue>




Node* NewNode(NodeType type,string val="");
void stripNewlines(vector<token>& tokens);

int find_token(token t,int start,const vector<token>& tokens);
int find_token_consecutive(token t,int start,int end,const vector<token>& tokens);

int match_token(int,int,TokenType,const vector<token>&);
int match_token_right(int,int,TokenType,const vector<token>&);

void delete_ast(Node* ast);
void copy_ast(Node*& dest,Node* src);

// Function to print AST in tablular form
void print_ast(Node* n,int spaces = 0);


char* clone_str(const char* str);
class Parser
{
private:
  vector<token> known_constants;
  vector<const char*> prefixes = {""};//for namespaces 
  refgraph* refGraph;
  string currSym;
  str_vector* files;
  str_vector* sources;
  string filename;
  size_t line_num = 1;
  int* num_of_constants;
  std::string aux;
  bool foundYield = false;
  //Context
  bool infunc;
  bool inclass;
  bool inloop;
  bool inif;
  bool inelif;
  bool inelse;
  bool intry;
  bool incatch;
  //in class method = inclass && infunc
  inline bool atGlobalLevel()
  {
    return (!infunc && !inclass && !inloop
     && !inif && !inelif && !inelse && !intry && !incatch);
  }
  inline bool isValidCtxForFunc()
  {
        return (!infunc  && !inloop
     && !inif && !inelif && !inelse && !intry && !incatch);
  }
  void parseError(string type,string msg);
  bool addSymRef(string name);
  Node* parseExpr(const vector<token>& tokens);
  Node* parseStmt(const vector<token>& tokens,size_t,size_t);
public:
  void set_source(zuko_src* p,size_t root_idx = 0); // for error printing and stuff
  Node* parse(const vector<token>& tokens,int begin,int end);
};
#endif
