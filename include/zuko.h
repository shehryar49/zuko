#ifndef ZUKO_H_
#define ZUKO_H_
#define ZUKO_INTERPRETER 
#define THREADED_INTERPRETER //ask vm to use threaded interpreter if possible
//not defining this macro will always result in the simple switch based interpret loop
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <unordered_map>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <cstdint>
#if defined(__linux__) || defined(__APPLE__)
    #include <dlfcn.h>
    #include <unistd.h>
    //For REPl
    #include <readline/readline.h>
    #include <readline/history.h>
#else
    #include <windows.h>
    #include <io.h>
    #include <fcntl.h>
#endif
#include "zapi.h"
using namespace std;
typedef ZObject(*BuiltinFunc)(ZObject*,int32_t);
void PromoteType(ZObject&,char t);
string fullform(char);
string ZObjectToStr(const ZObject&);
string substr(int32_t,int32_t,const string&);
int32_t find(char,string);
string replace_all(string,string,string);
string addlnbreaks(string,bool&);
unsigned char tobyte(const string&);
#include "opcode.h"
#include "overflow.h"
#include "convo.h"
#include "token.h"
#include "parser.h"
#include "compiler.h"
#include "builtinfunc.h"
#include "vm.h"
#include "lexer.h"
#include "utility.h"
#include "programinfo.h"
#include "repl.h"
#endif
