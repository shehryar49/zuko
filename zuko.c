#include "include/zuko.h"
#include <signal.h>

#define ZUKO_VER 0.3
#define ZUKO_VER_PATCH 3

void signalHandler(int signum)
{
    int x = 1;
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
           ZUKO_VER, ZUKO_VER_PATCH, __DATE__, __TIME__, get_os_name());
        REPL_init();
        repl();
        return 0;
    }
    char* source_code = NULL;
    const char *filename;
    if (argc >= 3 && strncmp(argv[1], "-c", 2) == 0)
    {
        filename = "argv";
        source_code = strdup(argv[2]);
    } 
    else
    {
        filename = argv[1];
        source_code = readfile(filename);
    }
    vm_init();
    zuko_src* src = create_source(filename, source_code);
    uint8_t* bytecode = NULL;
    size_t bytecode_len = 0;
    lexer_ctx lctx;
    token_vector tokens = tokenize(&lctx, src, true, 0, 0);
    if (lctx.hadErr) // had an error which was printed
        return 0;
    parser_ctx* pctx = create_parser_context(src); // create parser context and pass it the source we are working on
    Node* ast = parse_block(pctx, tokens.arr, 0,tokens.size - 1); // parse the tokens of root file
    // uncomment below line to print AST in tabular form
    //print_ast(ast,0);
    compiler_ctx* cctx = create_compiler_ctx(src);
    bytecode = compile_program(cctx, ast, argc, argv,0); // compile AST of program
    bytecode_len = cctx->bytes_done;
    //dis(bytecode);
    delete_ast(ast);
    parser_destroy(pctx);
    compiler_destroy(cctx);
    token_vector_destroy(&tokens);
    
    vm_load(bytecode, bytecode_len,src); // vm uses the src for printing errors and stuff
    // It's showtime
    interpret(0, true);
    vm_destroy();
    zuko_src_destroy(src);
    free(bytecode);
    // Hasta La Vista Baby
    return 0;
}
