#ifndef ZUKO_REPL_H_
#define ZUKO_REPL_H_

#ifdef __cplusplus
extern "C"{
#endif

// REPL STATE
extern bool REPL_MODE;
//Implementation
//EXPERIMENTAL!
void REPL_init();
void REPL();

#ifdef __cplusplus
}
#endif

#endif
