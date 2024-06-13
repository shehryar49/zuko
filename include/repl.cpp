#include "repl.h"
#include "compiler.h"
#include "programinfo.h"

bool REPL_MODE = false;
static ZukoSource src;
static int32_t k = 0;
static size_t stackSize = 0;//total globals added by VM initially
static Compiler compiler;
static Parser parser;

//Implementation
//EXPERIMENTAL!
void REPL()
{
  static string filename;
  k = src.files.size()+1;
  filename = "<stdin-"+to_string(k)+">";
  src.files.push_back(filename);
  src.sources.push_back("");
  
  compiler.reduceStackTo(stackSize);
  Lexer lex;
  string line;
  size_t offset = compiler.bytecode.size();
  vector<Token> tokens;
  Node* ast;
  bool continued = false;
  string prompt = ">>> ";
  while(true)
  {
    char* ptr;
    if(!continued)
      prompt = ">>> ";
    else
      prompt = "... ";
    #ifdef _WIN32
      line = REPL_READLINE(prompt.c_str());
    #else
    ptr = REPL_READLINE(prompt.c_str());
    if(!ptr)
    {
      puts("");
      exit(0);
    }
    line = ptr;
    if(ptr[0])
      add_history(ptr);
    free(ptr);  
    #endif

    if(line=="exit" || line=="quit" || line=="baskardebhai" || line=="yawr")
      exit(0);
    else if(line == ".showstack")
    {
      for(size_t i=0;i<vm.STACK.size;i++)
      {
        zobject e = vm.STACK.arr[i];
        printf("%s\n",zobjectToStr(e).c_str());
      }
      continue;
    }
    else if(line == ".showconstants")
    {
      printf("vm.total_constants = %d\n",vm.total_constants);
      for(int i=0;i<vm.total_constants;i++)
      {
        zobject e = vm.constants[i];
        printf("%s\n",zobjectToStr(e).c_str());
      }
      continue;
    }
    else if(line=="")
      continue;

    src.sources[k-1] += ((src.sources[k-1]=="") ? "" : "\n") +line;
    tokens = lex.generateTokens(src,true,k-1);
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
    parser.set_source(src,k-1);
    ast = parser.parse(tokens);

    zobject* constants = new zobject[src.num_of_constants];
    //copy previous constants
    for(int i=0;i<vm.total_constants;i++)
      constants[i] = vm.constants[i];
    if(vm.constants)
      delete[] vm.constants;
    vm.constants = constants;
    compiler.set_source(src,k-1);
    vector<uint8_t>& bytecode = compiler.compileProgram(ast,0,NULL,Compiler::OPT_COMPILE_DEADCODE); //ask the compiler to add previous bytecode before
    
    stackSize = compiler.globals.size();
    deleteAST(ast);
    
    vm.load(bytecode,src);
    vm.interpret(offset,false);//
    if(vm.STACK.size > stackSize)
        zlist_erase_range(&vm.STACK,stackSize,vm.STACK.size - 1);
    else if(vm.STACK.size < stackSize)
    {
      compiler.reduceStackTo(vm.STACK.size);
      stackSize = vm.STACK.size;
    }
    bytecode.pop_back();//pop OP_EXIT
    offset=bytecode.size();
    k = src.files.size()+1;
    filename = "<stdin-"+to_string(k)+">";
    src.files.push_back(filename);
    src.sources.push_back("");
  }
}