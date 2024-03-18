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
#ifndef Z_UTILITY_H_
#define Z_UTILITY_H_
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <unordered_map>
#include "zobject.h"
#include "zbytearray.h"
#include "zdict.h"
#include "programinfo.h"
std::string substr(int,int,const std::string&);
int len(std::string);


std::string IntToHex(int i);
unsigned char tobyte(const std::string& s);
int32_t hexToInt32(const std::string& s);
int64_t hexToInt64(const std::string& s);
std::string addlnbreaks(std::string s,bool& hadErr);
#ifdef _WIN32
string REPL_READLINE(const char* msg)
{
  signed char ch;
  string line;
  printf("%s",msg);
  while((ch = fgetc(stdin))!='\n')
  {
    if(ch == EOF) // readline is used with REPL, so on EOF (CTRL+D) we exit
    {
      puts("");
      exit(0);
    }
    line+=ch;
  }
  return line;
}
#else
  //use GNU Readline library
  #define REPL_READLINE readline
#endif
std::string& readfile(std::string filename);
void WriteByteCode(const char* fname,std::vector<uint8_t>& bytecode,std::unordered_map<size_t,ByteSrc>& LineNumberTable,std::vector<std::string>& files);
#ifdef PLUTONIUM_PROFILE
void showProfileInfo()
{
  int n =  sizeof(opNames)/sizeof(const char*);
  for(int i=1;i<n;i+=1)
  {
    for(int j=0;j<n-i;j++)
    {
      if(vm.instCount[j] < vm.instCount[j+1])
      {
        int c = vm.instCount[j];
        vm.instCount[j] = vm.instCount[j+1];
        vm.instCount[j+1] = c;
        const char* prev = opNames[j];
        opNames[j] = opNames[j+1];
        opNames[j+1] = prev;
      }
    }
  }
  printf("OPCODE  Count\n");
  for(int i=0;i<n;i++)
  {
    if(vm.instCount[i]!=0)
      printf("%s  %ld\n",opNames[i],vm.instCount[i]);
  }
}
#endif
const char* getOS();
std::string replace(int startpos,int endpos,std::string x,std::string s);
std::string replace_all(std::string x,std::string y,std::string s);//replaces all x strings in s with string y
std::string unescape(std::string s);
std::string ZObjectToStr(const ZObject& a);
std::string ZListToStr(ZList* p,std::vector<void*>* prev=nullptr);
std::string DictToStr(ZDict* p,std::vector<void*>* prev=nullptr);

#endif
