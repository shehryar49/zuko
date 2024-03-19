#ifndef ZUKO_H_
#define ZUKO_H_

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
#include "apiver.h"
#include "zobject.h"
#include "zstr.h"
#include "zbytearray.h"
#include "strmap.h"
#include "funobject.h"
#include "klass.h"
#include "klassobject.h"
#include "module.h"
#include "nativefun.h"
#include "coroutineobj.h"
#include "zlist.h"
#include "zdict.h"
#include "zfileobj.h"
#include "opcode.h"
#include "overflow.h"
#include "convo.h"
#include "token.h"
#include "parser.h"
#include "compiler.h"
#include "funobject.h"
#include "builtinfunc.h"
#include "vm.h"
#include "lexer.h"
#include "utility.h"
#include "programinfo.h"
#include "repl.h"



#endif
