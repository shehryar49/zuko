//#define PLUTONIUM_PROFILE
#include <signal.h>
#include <time.h>
#include <cstdint>
#include "include/plutonium.h"

#define PLUTONIUM_VER 0.3

void signalHandler(int signum)
{
  if(signum==SIGABRT || signum==SIGFPE || signum==SIGILL || signum==SIGSEGV)
  {
    char buff[] = "Oops either the interpreter or one of the loaded modules just crashed.Please report this incident.\n";
    #ifdef _WIN32
      _write(_fileno(stderr),buff,sizeof(buff));
    #else
      write(STDERR_FILENO,buff,sizeof(buff));
    #endif
    exit(EXIT_FAILURE);
  }
}


int main(int argc, const char* argv[])
{
    signal(SIGFPE,signalHandler);
    signal(SIGILL,signalHandler);
    signal(SIGABRT,signalHandler);
    signal(SIGSEGV,signalHandler);
    if (argc < 2)
    {
      printf("Plutonium Programming Langauge v%.2f build date(%s %s) %s\nCreated by Shahryar Ahmad\nREPL Mode(Experimental)\n", PLUTONIUM_VER,__DATE__,__TIME__,getOS());
      REPL();
      return 0;
    }
    string source_code;
    string filename;
    ProgramInfo p;
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
    //unless this 1 file imports other plutonium files
    //add this 1 file's name and source to ProgramInfo 
    p.files.push_back(filename);//add filename
    p.sources.push_back(source_code);//add source
    //parser can add more files to the program by adding them in ProgramInfo
     
    vector<uint8_t> bytecode;
    //the variables in curly brackets below are temporary
    //they are not needed during program execution
    {
        Lexer lex;
        vector<Token> tokens = lex.generateTokens(filename,source_code);
        if(tokens.size()==0)//empty program nothing to do
          return 0;
        
        Parser parser;
        parser.init(filename,p);//init parser with root filename and ProgramInfo
        Node* ast = parser.parse(tokens); //parse the tokens of root file
        //uncomment below line to print AST in tabular form
      //  printAST(ast);
        Compiler compiler;
        compiler.init(filename,p);//init compiler with root filename and ProgramInfo
        bytecode = compiler.compileProgram(ast,argc,argv); // compile AST of program
        deleteAST(ast);
    }
   // WriteByteCode(bytecode,LineNumberTable,files);
    
    vm.load(bytecode,p);
   
    //it's showtime
    vm.interpret();
    #ifdef PLUTONIUM_PROFILE
     showProfileInfo();
    #endif
    return 0;
}
