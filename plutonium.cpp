//#define PLUTONIUM_PROFILE
#include <signal.h>
#include <time.h>
#include <cstdint>
#include "include/plutonium.h"

#define PLUTONIUM_VER 0.3
bool REPL_MODE = false;
void signalHandler(int signum)
{
  if(signum==SIGABRT || signum==SIGFPE || signum==SIGILL || signum==SIGSEGV)
  {
    char buff[] = "Oops either the interpreter or one of the loaded modules just crashed.Please report this incident.\n";
    #ifdef _WIN32
      _write(STDERR_FILENO,buff,sizeof(buff));
    #else
      write(STDERR_FILENO,buff,sizeof(buff));
    #endif
    exit(EXIT_FAILURE);
  }
}

void REPL()
{
  REPL_MODE = true;
  static vector<string> sources;
  static vector<string> files;
  static string filename;
  static int32_t k = 0;
  k = files.size()+1;
  filename = "<stdin-"+to_string(k)+">";
  files.push_back(filename);
  sources.push_back("");
  static std::unordered_map<size_t,ByteSrc> LineNumberTable;
  static size_t stackSize = 0;
  static Compiler compiler;
  if(vm.STACK.size() > stackSize)
  {
    vm.STACK.erase(vm.STACK.begin()+stackSize,vm.STACK.end());
  }
  compiler.reduceStackTo(stackSize);
  unordered_map<string,bool> symRef;
  Lexer lex;
  string line;
  size_t offset = compiler.bytecode.size();
  vector<Token> tokens;
  Node* ast;
  static Parser parser;
  static ParseInfo p;
  bool continued = false;

  while(true)
  {
    if(!continued)
      printf(">>> ");
    else
      printf("... ");
    line = readline();
    if(line=="exit" || line=="quit" || line=="baskardebhai" || line=="yawr")
      exit(0);
    else if(line == ".showstack")
    {
      for(auto e: vm.STACK)
        printf("%s\n",PltObjectToStr(e).c_str());
      continue;
    }
    else if(line == ".showconstants")
    {
      printf("vm.total_constants = %d\n",vm.total_constants);
      for(int i=0;i<vm.total_constants;i++)
      {
        PltObject e = vm.constants[i];
        printf("%s\n",PltObjectToStr(e).c_str());
      }
      continue;
    }
    
    else if(line=="")
      continue;
    sources[k-1] += ((sources[k-1]=="") ? "" : "\n") +line;
    tokens = lex.generateTokens(filename,sources[k-1]);
    int i1=0;
    for(auto tok: tokens)
    {
      if(tok.type==L_CURLY_BRACKET_TOKEN)
        i1+=1;
      else if(tok.type==R_CURLY_BRACKET_TOKEN)
        i1-=1;
    }
    if(i1!=0 )
    {
      continued=true;
      continue;
    }
    continued = false;
    parser.init(p,&files,&sources,filename);

    ast = parser.parse(tokens);
    PltObject* constants = new PltObject[p.num_of_constants];
    //copy previous constants
    for(int i=0;i<vm.total_constants;i++)
      constants[i] = vm.constants[i];
    if(vm.constants)
      delete[] vm.constants;
    vm.constants = constants;
    compiler.init(p,&files,&sources,&LineNumberTable,filename);
    vector<uint8_t>& bytecode = compiler.compileProgram(ast,0,NULL,true,false); //ask the compiler to add previous bytecode before
    stackSize = vm.STACK.size();
    deleteAST(ast);
    
    vm.load(bytecode,&LineNumberTable,&files,&sources);
   // WriteByteCode(bytecode,LineNumberTable,files);// for debugging
    vm.interpret(offset,false);//
    //if the instruction executed successfuly
    //it might have changed the stack size 
    stackSize = vm.STACK.size();
    bytecode.pop_back();//pop OP_EXIT
    offset=bytecode.size();
    k=files.size()+1;
    filename = "<stdin-"+to_string(k)+">";
    files.push_back(filename);
    sources.push_back("");
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
    std::unordered_map<size_t, ByteSrc> LineNumberTable;
    vector<string> files;// filenames
    vector<string> sources;
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
    files.push_back(filename);//add to the list of compiled files

    vector<uint8_t> bytecode;

    sources.push_back(source_code);
    {
        Lexer lex;
        vector<Token> tokens = lex.generateTokens(filename,source_code);
        if(tokens.size()==0)//empty program nothing to do
          return 0;
        ParseInfo p;
        /*
        ParseInfo stores things like reference graph, number of constants etc
        */
        Parser parser;
        //the parser handles file imports so address of two  string vectors are passed
        //initially files vector has only 1 name i.e the main file but parser can add to it
        //the sources vector also has initially 1 element but the parser adds to it as it reads more files
        
        parser.init(p,&files,&sources,filename);
        Node* ast = parser.parse(tokens);
        //printAST(ast);
        Compiler compiler;
        compiler.init(p,&files,&sources,&LineNumberTable,filename);
        bytecode = compiler.compileProgram(ast,argc,argv);
        deleteAST(ast);

    }
    //WriteByteCode(bytecode,LineNumberTable,files);
    vm.load(bytecode,&LineNumberTable,&files,&sources);
    
    vm.interpret();
    #ifdef PLUTONIUM_PROFILE
     showProfileInfo();
    #endif
    return 0;
}
