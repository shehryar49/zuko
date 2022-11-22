#ifndef PLUTONIUM_H_
#define PLUTONIUM_H_
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
#ifdef BUILD_FOR_LINUX
#include <dlfcn.h>
#include <unistd.h>
#else
#include <windows.h>
#endif
using namespace std;
#include "PltObject.h"
#define PltArgs const vector<PltObject>&
typedef PltObject(*BuiltinFunc)(PltObject*,int);
void PromoteType(PltObject&,char t);
string fullform(char);
string PltObjectToStr(const PltObject&);
string substr(int,int,string);
int find(char,string);
string replace_all(string,string,string);
string addlnbreaks(string,bool&);
unsigned char tobyte(string);
#include "opcode.h"
#include "overflow.h"
#include "unions.h"
#include "convo.h"
#include "token.h"
#include "parser.h"
#include "compiler.h"
#include "builtinfunc.h"
#include "vm.h"
#include "lexer.h"
#include "var.h"
#include "utility.h"
#endif
