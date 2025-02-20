#include "include/zuko.h"
#include "include/signal-handlers.h"


int main(int argc, const char *argv[])
{
    setup_signal_handlers();
    if (argc < 2)
    {
        repl_init();
        repl();
        return 0;
    }
    char* source_code = NULL;
    const char* filename;
    if (argc >= 3 && strncmp(argv[1], "-c", 2) == 0)
    {
        filename = "argv";
        source_code = strdup(argv[2]);
    } 
    else
    {
        filename = argv[1];
        source_code = readfile(filename);
        if(!source_code)
        {
            printf("Error opening file: %s\n",strerror(errno));
            return 1;
        }
    }
    vm_init();
    zuko_src* src = create_source(filename, source_code);
    uint8_t* bytecode = NULL;
    size_t bytecode_len = 0;
    lexer_ctx lctx;
    token_vector tokens = tokenize(&lctx, src, true, 0, 0);
    if (lctx.had_error) // had an error which was printed
        return 0;
    parser_ctx* pctx = create_parser_context(src); // create parser context and pass it the source we are working on
    Node* ast = parse_block(pctx, tokens.arr, 0,tokens.size - 1); // parse the tokens of root file
    // uncomment below line to print AST in tabular form
    //print_ast(ast,0);
    compiler_ctx* cctx = create_compiler_context(src);
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
