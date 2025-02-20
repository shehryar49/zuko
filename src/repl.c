#include "dyn-str.h"
#include "misc.h"
#include "parser.h"
#include "token-vector.h"
#include "token.h"
#include "vm.h"
#include "zuko-src.h"
#include "compiler.h"
#include "lexer.h"
#include <stdint.h>
#include "dis.h"
#include "repl.h"
#include "zuko-ver.h"

bool REPL_MODE = false;

zuko_src* src;
size_t stack_size = 19;//total globals added by compiler initially
parser_ctx* pctx;
compiler_ctx* cctx;
dyn_str text;
size_t text_size_processed = 0;
size_t vm_offset = 0;
lexer_ctx lex;

void repl_init() {
    REPL_MODE = true;
    vm_init();
    dyn_str_init(&text);
    src = create_source("<stdin>",text.arr);
    pctx = create_parser_context(src);
    cctx = create_compiler_context(src);
    printf("Zuko Programming Langauge v%d.%d.%d build date(%s %s) %s\nCreated by Shahryar Ahmad\nREPL Mode(Experimental)\n",
           ZUKO_VER_MAJOR,ZUKO_VER_MINOR,ZUKO_VER_PATCH, __DATE__, __TIME__, get_os_name());

}
//Implementation
void repl() {
    const char* prompt = ">>> ";
    bool continued = false;
    compiler_reduce_stack_size(cctx, stack_size);
    text_size_processed = text.length;
    while(true) {
        if(continued)
            prompt = "... ";
        char* line = REPL_READLINE(prompt);
        if(!line) { // EOF
            puts("");
            break;
        }
        if(strcmp(line,"yawr") == 0 || strcmp(line,"quit") == 0) {
            free(line);
            puts("");
            break;
        }
        else if(strcmp(line,".showstack") == 0) {
            for(size_t i = 0; i < STACK.size; i++) {
                print_zobject(STACK.arr[i]);
                putc('\n',stdout);
            }
            free(line);
            continue;
        }
        else if(strcmp(line,".showtext") == 0)
        {
            puts(text.arr);
            free(line);
            continue;
        }
        else if(strcmp(line,"") == 0) {
            free(line);
            continue;
        }
        #ifndef _WIN32
        if(line[0])
            add_history(line);
        #endif
        dyn_str_append(&text, line);
        dyn_str_push(&text, '\n');
        free(line);
        
        src->sources.arr[0] = text.arr; // jugaad
        token_vector tokens = tokenize(&lex,src,true,0,text_size_processed);
        int32_t i1 = 0;
        int32_t i2 = 0;
        int32_t i3 = 0;
        for(size_t i = 0; i < tokens.size; i++) {
            token curr = tokens.arr[i];
            if(curr.type == LParen_TOKEN)
                i1++;
            if(curr.type == L_CURLY_BRACKET_TOKEN)
                i2++;
            if(curr.type == BEGIN_LIST_TOKEN)
                i3++;

            if(curr.type == RParen_TOKEN)
                i1--;
            if(curr.type == R_CURLY_BRACKET_TOKEN)
                i2--;
            if(curr.type == END_LIST_TOKEN)
                i3--;
        }
        if(i1 != 0 || i2!= 0 || i3 != 0) {
            token_vector_destroy(&tokens);
            continued = true;
            continue;
        }
        continued = false;
        prompt = ">>> ";

        //parse and execute
        text_size_processed = text.length;
        Node* ast = parse_block(pctx,tokens.arr, 0, tokens.size - 1);
        token_vector_destroy(&tokens);
        uint8_t* bytecode = compile_program(cctx, ast, 0, NULL, OPT_COMPILE_DEADCODE | OPT_NOPOP_GLOBALS);
        vm_load(bytecode,cctx->bytes_done,src);
        interpret(vm_offset, false);
        stack_size = STACK.size;
        //get rid of OP_EXIT
        cctx->bytes_done--;
        cctx->bytecode.size--;
        vm_offset = cctx->bytecode.size;
    }
    free(cctx->bytecode.arr);
    zuko_src_destroy(src);
    parser_destroy(pctx);
    compiler_destroy(cctx);
    vm_destroy();    
}
