#include "include/zuko.h"
#include <signal.h>

#define ZUKO_VER 0.3
#define ZUKO_VER_PATCH 3

void signalHandler(int signum)
{
    if (signum == SIGABRT || signum == SIGFPE || signum == SIGILL || signum == SIGSEGV)
    {
        char buff[] = "Oops! Either the interpreter or one of the loaded modules just crashed. Please report this incident.\n";
        #ifdef _WIN32
            size_t written = _write(_fileno(stderr), buff, sizeof(buff));
        #else
            size_t written __attribute__((unused)) =
            write(STDERR_FILENO, buff, sizeof(buff));
        #endif
        exit(EXIT_FAILURE);
    }  
}

int main(int argc, const char *argv[])
{
    // Handle crashes
    // Very unlikely but ...
    // Shit happens
    signal(SIGFPE, signalHandler);
    signal(SIGILL, signalHandler);
    signal(SIGABRT, signalHandler);
    signal(SIGSEGV, signalHandler);
    if (argc < 2)
    {
        printf("Zuko Programming Langauge v%.1f.%i build date(%s %s) %s\nCreated by Shahryar Ahmad\nREPL Mode(Experimental)\n",
           ZUKO_VER, ZUKO_VER_PATCH, __DATE__, __TIME__, getOS());
        REPL_init();
        REPL();
        return 0;
    }
    char *source_code = NULL;
    const char *filename;
    if (argc >= 3 && strncmp(argv[1], "-c", 2) == 0)
    {
        filename = clone_str("argv");
        source_code = clone_str(argv[2]);
    } 
    else
    {
        filename = argv[1];
        source_code = readfile(filename);
    }
    zuko_src *src = create_source(filename, source_code);
    uint8_t *bytecode;
    size_t bytecode_len;
    // the variables in curly brackets below are temporary
    // they are not needed during program execution
    {
        lexer_ctx lex;
        token_vector tokens = tokenize(&lex, src, true, 0);
        if (lex.hadErr) // had an error which was printed
            return 0;
        parser_ctx *pctx = create_parser_ctx(src);
        Node *ast = parse_block(pctx, tokens.arr, 0,tokens.size - 1); // parse the tokens of root file
        // uncomment below line to print AST in tabular form
        // print_ast(ast,0);
        vm_init();
        compiler_ctx *cctx = create_compiler_ctx(src);
        bytecode = compile_program(cctx, ast, argc, argv,OPT_POP_GLOBALS); // compile AST of program
        bytecode_len = cctx->bytes_done;
        // dis(bytecode);
        delete_ast(ast);
        parser_destroy(pctx);
        compiler_destroy(cctx);
        token_vector_destroy(&tokens);
    }
    vm_load(bytecode, bytecode_len,src); // vm uses the src for printing errors and stuff
    // It's showtime
    interpret(0, true);
    vm_destroy();
    zuko_src_destroy(src);
    free(bytecode);
    // Hasta La Vista Baby
    return 0;
}
