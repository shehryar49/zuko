#ifndef PLUTONIUM_H_
#define PLUTONIUM_H_
#define PLUTONIUM_INTERPRETER
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
#ifdef __linux__
    #include <dlfcn.h>
    #include <unistd.h>
#else
    #include <windows.h>
    #include <io.h>
    #include <fcntl.h>
#endif
#include "PltObject.h"
using namespace std;
typedef PltObject(*BuiltinFunc)(PltObject*,int32_t);
void PromoteType(PltObject&,char t);
string fullform(char);
string PltObjectToStr(const PltObject&);
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
