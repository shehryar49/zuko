#include <stdint.h>
#include <signal.h>
#include "include/zuko.h"
#include "compiler.h"
#include "parser.h"
#include "token.h"
#include "zuko-src.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "misc.h"
#define ZUKO_VER 0.3
#define ZUKO_VER_PATCH 3

void signalHandler(int signum)
{
    if(signum==SIGABRT || signum==SIGFPE || signum==SIGILL || signum==SIGSEGV)
    {
        char buff[] = "Oops! Either the interpreter or one of the loaded modules just crashed. Please report this incident.\n";
        #ifdef _WIN32
            size_t written = _write(_fileno(stderr),buff,sizeof(buff));
        #else
            size_t written __attribute__((unused)) = write(STDERR_FILENO,buff,sizeof(buff));
        #endif
        exit(EXIT_FAILURE);
    }
}

int main(int argc, const char* argv[])
{
    // Handle crashes
    // Very unlikely but ...
    // Shit happens
    signal(SIGFPE,signalHandler);
    signal(SIGILL,signalHandler);
    signal(SIGABRT,signalHandler);
    signal(SIGSEGV,signalHandler);
    if (argc < 2)
    {
        printf("Zuko Programming Langauge v%.1f.%i build date(%s %s) %s\nCreated by Shahryar Ahmad\nREPL Mode(Experimental)\n", ZUKO_VER,ZUKO_VER_PATCH,__DATE__,__TIME__,getOS());
        REPL_init();
        REPL();
        return 0;
    }
    char* source_code = NULL;
    char* filename;
    if(argc>=3 && strncmp(argv[1],"-c",2)==0)
    {
        filename = clone_str("argv");
        source_code = clone_str(argv[2]);
    }
    else
    {
        filename = clone_str(argv[1]);
        source_code = readfile(filename);
    }
    zuko_src src;
    zuko_src_init(&src);
    zuko_src_add_file(&src, filename,source_code);

     
    uint8_t* bytecode;
    size_t bytecode_len;
    //the variables in curly brackets below are temporary
    //they are not needed during program execution
    {
        lexer lex;
        token_vector tokens = lexer_generateTokens(&lex,&src,true,0);
        if(lex.hadErr)//had an error which was printed
          return 0;
        Parser parser;
        parser_init(&parser);
        parser_set_source(&parser,&src,0);
        Node* ast = parse_block(&parser,tokens.arr,0,tokens.size-1); //parse the tokens of root file

        //uncomment below line to print AST in tabular form
        //print_ast(ast,0);
        vm_init();
        compiler comp;
        compiler_set_source(&comp,&src,0);//init compiler with ZukoSource
        bytecode = compile_program(&comp,ast,argc,argv,OPT_POP_GLOBALS); // compile AST of program
        bytecode_len = comp.bytes_done;
        //dis(bytecode);
        delete_ast(ast);
       // for(size_t i=0;i<tokens.size;i++)
        //{
          //  if(tokens.arr[i].type == STRING_TOKEN)
         //       free((void*)tokens.arr[i].content);
        //}
        parser_destroy(&parser);
        compiler_destroy(&comp);
    }

    vm_load(bytecode,bytecode_len,&src); // vm uses the src for printing errors and stuff

    // It's showtime
    interpret(0,true);
    vm_destroy();
    zuko_src_destroy(&src);
    // Hasta La Vista Baby    
    return 0;
}
