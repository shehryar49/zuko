#include <signal.h>
#include "include/zuko.h"


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
      REPL_MODE = true; //enable REPL_MODE
      printf("Zuko Programming Langauge v%.1f.%i build date(%s %s) %s\nCreated by Shahryar Ahmad\nREPL Mode(Experimental)\n", ZUKO_VER,ZUKO_VER_PATCH,__DATE__,__TIME__,getOS());
      REPL();
      return 0;
    }
    string source_code;
    string filename;
    if(argc>=3 && strncmp(argv[1],"-c",2)==0)
    {
      filename = "argv";
      source_code = argv[2];
    }
    else
    {
      filename = argv[1];
      source_code = readfile(filename);
    }
    //the  program currently consists of 1 file only
    //unless this 1 file imports other zuko files
    ZukoSource src;
    src.addFile(filename,source_code); //first file is the root file
    //parser can add more files to the program
     
    vector<uint8_t> bytecode;
    //the variables in curly brackets below are temporary
    //they are not needed during program execution
    {
        Lexer lex;
        vector<Token> tokens = lex.generateTokens(src);
        if(lex.hadErr)//had an error which was printed
          return 0;
        
        Parser parser;
        parser.set_source(src);
        Node* ast = parser.parse(tokens); //parse the tokens of root file

        //uncomment below line to print AST in tabular form
        //printAST(ast);
        
        Compiler compiler;
        compiler.set_source(src);//init compiler with ZukoSource
        bytecode = compiler.compileProgram(ast,argc,argv); // compile AST of program
        deleteAST(ast);
    }
    vm.load(bytecode,src); // vm uses the src for printing errors and stuff
    // It's showtime
    vm.interpret();
    // Hasta La Vista Baby    
    return 0;
}
