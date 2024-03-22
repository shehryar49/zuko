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
//#include "utility.h"
#include <string>
#include <vector>
#include "token.h"





class Lexer
{
private:
  std::string filename;
  std::string source_code;
  size_t srcLen;
  
  bool printErr = true;

  static bool isKeyword(std::string s);
  static const char* keywords[];
  static Token resolveMacro(const std::string& name);

  void str(size_t&,std::vector<Token>&);
  void macro(size_t&,std::vector<Token>&);
  void comment(size_t&,std::vector<Token>&);
  void numeric(size_t&,std::vector<Token>&);
  void id(size_t&,std::vector<Token>&);

  void lexErr(std::string type,std::string msg);
public:
  size_t line_num;  
  //exceptions can be expensive so ...
  bool hadErr = false;
  std::string errmsg;
  ///
  vector<Token> generateTokens(string fname,const string& s,bool printErr = true);
};
#endif // LEXER_H_
