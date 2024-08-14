#ifndef ZUKO_H_
#define ZUKO_H_
//This header should only be included by the zuko interpreter


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

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
#include "misc.h"
#include "zuko-src.h"
#include "repl.h"
#include "dis.h"


#endif
