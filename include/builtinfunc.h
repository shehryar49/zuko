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


//This header contains some builtin functions which are embed into the interpreter

#ifndef BUILTIN_FUNC_H_
#define BUILTIN_FUNC_H_
#include "zobject.h"
#include <vector>
#include <unordered_map>
#include <string>
typedef ZObject(*BuiltinFunc)(ZObject*,int32_t);

ZObject Z_ISALPHA(ZObject* args,int32_t argc);
ZObject ASCII(ZObject* args,int32_t argc);
ZObject TOCHAR(ZObject* args,int32_t argc);
void printZDict(ZDict* l,std::vector<void*> seen = {});
void printList(ZList* l,std::vector<void*> seen = {});
void printByteArray(ZByteArr* arr);
ZObject print(ZObject* args,int32_t argc);
ZObject PRINTF(ZObject* args,int32_t argc);
ZObject FORMAT(ZObject* args,int32_t argc);
ZObject println(ZObject* args,int32_t argc);
ZObject input(ZObject* args,int32_t argc);
ZObject TYPEOF(ZObject* args,int32_t argc);
ZObject isInstanceOf(ZObject* args,int32_t argc);
ZObject LEN(ZObject* args,int32_t argc);
ZObject OPEN(ZObject* args,int32_t argc);
ZObject READ(ZObject* args,int32_t argc);
ZObject CLOSE(ZObject* args,int32_t argc);
ZObject RAND(ZObject* args,int32_t argc);
ZObject BYTEARRAY(ZObject* args,int32_t argc);
ZObject WRITE(ZObject* args,int32_t argc);
ZObject EXIT(ZObject* args,int32_t argc);
ZObject REVERSE(ZObject* args,int32_t argc);
ZObject BYTES(ZObject* args,int32_t argc);
ZObject OBJINFO(ZObject* args,int32_t argc);
ZObject moduleInfo(ZObject* args,int32_t argc);
/*union FOO
{
  int32_t x;
  unsigned char bytes[sizeof(x)];
}FOO;
union FOO1
{
  int64_t l;
  unsigned char bytes[sizeof(l)];
}FOO1;
union FOO2
{
  double f;
  unsigned char bytes[sizeof(f)];
}FOO2;
*/
///////
/////////

ZObject SUBSTR(ZObject* args,int32_t argc);
ZObject getFileSize(ZObject* args,int32_t argc);
ZObject FTELL(ZObject* args,int32_t argc);
ZObject REWIND(ZObject* args,int32_t argc);
ZObject SYSTEM(ZObject* args,int32_t argc);
ZObject SPLIT(ZObject* args,int32_t argc);
ZObject GETENV(ZObject* args,int32_t argc);
ZObject SHUFFLE(ZObject* args,int32_t argc);
ZObject STR(ZObject* args,int32_t argc);
ZObject FIND(ZObject* args,int32_t argc);
ZObject TOINT(ZObject* args,int32_t argc);
ZObject TOINT32(ZObject* args,int32_t argc);
ZObject TOINT64(ZObject* args,int32_t argc);
ZObject TOFLOAT(ZObject* args,int32_t argc);
ZObject tonumeric(ZObject* args,int32_t argc);
ZObject isnumeric(ZObject* args,int32_t argc);
ZObject REPLACE(ZObject* args,int32_t argc);
ZObject REPLACE_ONCE(ZObject* args,int32_t argc);
ZObject SLEEP(ZObject* args,int32_t argc);

ZObject TOBYTE(ZObject* args,int32_t argc);
ZObject writelines(ZObject* args,int32_t argc);
ZObject readlines(ZObject* args,int32_t argc);
void clean_stdin(void);
ZObject FREAD(ZObject* args,int32_t argc);
ZObject FWRITE(ZObject* args,int32_t argc);
ZObject FSEEK(ZObject* args,int32_t argc);
//
ZList* makeListCopy(ZList);
ZDict* makeDictCopy(ZDict);

ZObject COPY(ZObject* args,int32_t argc);
ZObject POW(ZObject* args,int32_t argc);
ZObject CLOCK(ZObject* args,int32_t argc);
////////////////////
//Builtin Methods
//Methods work exactly like functions but their first argument should always
//be an object
//these are just functions that work on multiple supported types
//for example POP functions works on both list,bytearrays and mutable strings
ZObject POP(ZObject* args,int32_t argc);
ZObject CLEAR(ZObject* args,int32_t argc);
ZObject PUSH(ZObject* args,int32_t argc);
ZObject FIND_METHOD(ZObject* args,int32_t argc);
ZObject INSERTBYTEARRAY(ZObject* args,int32_t argc);
ZObject INSERTSTR(ZObject* args,int32_t argc);
ZObject INSERT(ZObject* args,int32_t argc);
ZObject ERASE(ZObject* args,int32_t argc);
ZObject asMap(ZObject* args,int32_t argc);
ZObject ASLIST(ZObject* args,int32_t argc);
ZObject REVERSE_METHOD(ZObject* args,int32_t argc);
ZObject EMPLACE(ZObject* args,int32_t argc);
ZObject HASKEY(ZObject* args,int32_t argc);
ZObject UNPACK(ZObject* args,int32_t argc);
//String methods
ZObject SUBSTR_METHOD(ZObject* args,int32_t argc);
ZObject REPLACE_METHOD(ZObject* args,int32_t argc);
ZObject REPLACE_ONCE_METHOD(ZObject* args,int32_t argc);

////////////////////
///////////////
void initFunctions();
void initMethods();
ZObject callmethod(std::string name,ZObject* args,int32_t argc);
bool function_exists(std::string name);

#endif
