#ifndef ZUKO_REPL_H_
#define ZUKO_REPL_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <stdbool.h>
extern bool REPL_MODE;
void REPL_init();
void REPL();
void repl();
#ifdef __cplusplus
}
#endif

#endif
