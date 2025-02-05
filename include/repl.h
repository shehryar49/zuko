#ifndef ZUKO_REPL_H_
#define ZUKO_REPL_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <stdbool.h>

#ifndef _WIN32
    #include <readline/readline.h>
    #include <readline/history.h>
#endif

extern bool REPL_MODE;
void repl_init();
void repl();

#ifdef __cplusplus
}
#endif

#endif
