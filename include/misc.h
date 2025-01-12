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
#ifndef Z_MISC_H_
#define Z_MISC_H_
#include <stdio.h>
#include <stdlib.h>
#include "ptr-vector.h"
#include "zobject.h"
#include "zbytearray.h"
#include "zdict.h"
#include "dyn-str.h"
#ifdef __cplusplus
extern "C"{
#endif

char* readfile(const char* filename); /* Take a guess */
char* zlist_to_str(zlist*,ptr_vector*); /* Returns a string representation of a zuko list */
char* zdict_to_str(zdict*,ptr_vector*); /* Returns a string representation of a zuko dictionary */

const char* get_os_name(); /* Returns OS name */
char* zobject_to_str(zobject);
unsigned char tobyte(const char*);
void replace_once(zstr*,zstr*,zstr*,dyn_str*);
void replace_all(zstr*,zstr*,zstr*,dyn_str*);
#ifdef __cplusplus
}
#endif

//int len(std::string s);
//std::string substr(int x,int y,const std::string& s);
/*std::vector<std::string> split(std::string s,const std::string& x);
std::string lstrip(std::string s);
std::string replace(std::string x,std::string y,std::string s);//Replaces only once

std::string IntToHex(int i);
unsigned char tobyte(const std::string& s);
int32_t hexToInt32(const std::string& s);
int64_t hexToInt64(const std::string& s);
std::string addlnbreaks(std::string s,bool& hadErr);
*/
#ifdef _WIN32
string REPL_READLINE(const char* msg);
#else
  //use GNU Readline library
  #define REPL_READLINE readline
#endif


/*std::string replace(int startpos,int endpos,std::string x,std::string s);
std::string replace_all(std::string x,std::string y,std::string s);//replaces all x strings in s with string y
std::string unescape(std::string s);
std::string zobjectToStr(const zobject& a);
std::string ZListToStr(zlist* p,std::vector<void*>* prev=nullptr);
std::string DictToStr(zdict* p,std::vector<void*>* prev=nullptr);*/

#endif
