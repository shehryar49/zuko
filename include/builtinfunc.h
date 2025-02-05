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
#include "builtin-map.h"
#include "ptr-vector.h"

#ifdef __cplusplus
extern "C"{
#endif

typedef zobject(*BuiltinFunc)(zobject*,int32_t);

//Regular functions
//All their names are in uppercase, picked this naming convention for no particular reason
zobject Z_ISALPHA(zobject* args,int32_t argc);
zobject ASCII(zobject* args,int32_t argc);
zobject TOCHAR(zobject* args,int32_t argc);
zobject PRINT(zobject* args,int32_t argc);
zobject PRINTF(zobject* args,int32_t argc);
zobject FORMAT(zobject* args,int32_t argc);
zobject PRINTLN(zobject* args,int32_t argc);
zobject ZUKO_INPUT(zobject* args,int32_t argc); // fuck Visual Studio for not letting me use "INPUT"
zobject TYPEOF(zobject* args,int32_t argc);
zobject ISINSTANCEOF(zobject* args,int32_t argc);
zobject LEN(zobject* args,int32_t argc);
zobject OPEN(zobject* args,int32_t argc);
zobject READ(zobject* args,int32_t argc);
zobject CLOSE(zobject* args,int32_t argc);
zobject RAND(zobject* args,int32_t argc);
zobject BYTEARRAY(zobject* args,int32_t argc);
zobject WRITE(zobject* args,int32_t argc);
zobject EXIT(zobject* args,int32_t argc);
zobject REVERSE(zobject* args,int32_t argc);
zobject BYTES(zobject* args,int32_t argc);
zobject OBJINFO(zobject* args,int32_t argc);
zobject MODULEINFO(zobject* args,int32_t argc);
zobject SUBSTR(zobject* args,int32_t argc);
zobject GETFILESIZE(zobject* args,int32_t argc);
zobject FTELL(zobject* args,int32_t argc);
zobject REWIND(zobject* args,int32_t argc);
zobject SYSTEM(zobject* args,int32_t argc);
zobject SPLIT(zobject* args,int32_t argc);
zobject GETENV(zobject* args,int32_t argc);
zobject STR(zobject* args,int32_t argc);
zobject FIND(zobject* args,int32_t argc);
zobject TOINT(zobject* args,int32_t argc);
zobject TOINT32(zobject* args,int32_t argc);
zobject TOINT64(zobject* args,int32_t argc);
zobject TOFLOAT(zobject* args,int32_t argc);
zobject tonumeric(zobject* args,int32_t argc);
zobject isnumeric(zobject* args,int32_t argc);
zobject REPLACE(zobject* args,int32_t argc);
zobject REPLACE_ONCE(zobject* args,int32_t argc);
zobject ZUKO_SLEEP(zobject* args,int32_t argc);
zobject TOBYTE(zobject* args,int32_t argc); //the byte() type conversion function, couldn't use that name here
zobject WRITELINES(zobject* args,int32_t argc);
zobject READLINES(zobject* args,int32_t argc);
void clean_stdin(void);
zobject FREAD(zobject* args,int32_t argc);
zobject FWRITE(zobject* args,int32_t argc);
zobject FSEEK(zobject* args,int32_t argc);
zobject FFLUSH(zobject* args,int32_t argc);
//
zlist* makeListCopy(zlist);
zdict* makeDictCopy(zdict);

zobject COPY(zobject* args,int32_t argc);
zobject CLOCK(zobject* args,int32_t argc);
////////////////////
//Builtin Methods
//Methods work exactly like functions but their first argument should always
//be an object
//these are just functions that work on multiple supported types
//for example POP function works on both lists and bytearrays
zobject POP(zobject* args,int32_t argc);
zobject CLEAR(zobject* args,int32_t argc);
zobject PUSH(zobject* args,int32_t argc);
zobject FIND_METHOD(zobject* args,int32_t argc);
zobject INSERTBYTEARRAY(zobject* args,int32_t argc);
zobject INSERTSTR(zobject* args,int32_t argc);
zobject INSERT(zobject* args,int32_t argc);
zobject ERASE(zobject* args,int32_t argc);
zobject ASMAP(zobject* args,int32_t argc);
zobject ASLIST(zobject* args,int32_t argc);
zobject REVERSE_METHOD(zobject* args,int32_t argc);
zobject EMPLACE(zobject* args,int32_t argc);
zobject HASKEY(zobject* args,int32_t argc);
zobject UNPACK(zobject* args,int32_t argc);
//Only for string methods
zobject SUBSTR_METHOD(zobject* args,int32_t argc);
zobject REPLACE_METHOD(zobject* args,int32_t argc);
zobject REPLACE_ONCE_METHOD(zobject* args,int32_t argc);
///////////////

extern bmap funcs;
//Following 2 functions populate hashmaps with the addresses of all the above functions
void init_builtin_functions();
void init_builtin_methods();
//Calls a method after a hashtable lookup
zobject callmethod(const char* name,zobject* args,int32_t argc);
bool function_exists(const char* name);

#ifdef __cplusplus
}
#endif

#endif
