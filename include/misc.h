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
#include "builtinfunc.h"
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
int32_t hex_to_int32(const char*);
int64_t hex_to_int64(const char*);
uint8_t hex_to_uint8(const char*);

const char* get_os_name(); /* Returns OS name */

void print_zlist(zlist*,ptr_vector*);
void print_zdict(zdict*,ptr_vector*);
void print_zbytearray(zbytearr *arr);
void print_zobject(zobject);

char* zobject_to_str(zobject);

void replace_once(zstr*,zstr*,zstr*,dyn_str*);
void replace_all(zstr*,zstr*,zstr*,dyn_str*);

void run_zuko_file(const char* filename,int argc,const char** argv);
void run_zuko_code(const char* filename,char* code,int argc,const char** argv);

#ifdef __cplusplus
}
#endif

#ifdef _WIN32
    char* REPL_READLINE(const char* msg);
#else
    //use GNU Readline library
    #define REPL_READLINE readline
#endif



#endif
