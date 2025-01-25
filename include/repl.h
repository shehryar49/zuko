#ifndef ZUKO_REPL_H_
#define ZUKO_REPL_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <stdbool.h>
extern bool REPL_MODE;
void repl_init();
void repl();
#ifdef __cplusplus
}
#endif

#endif
