#include "parser.h"
#include "zuko-src.h"
#include "zuko.h"
#include <readline/readline.h>

bool REPL_MODE = false;
static zuko_src src;
static int32_t k = 0;
static size_t stackSize = 0;//total globals added by VM initially
static Compiler compiler;
static Parser parser;

void REPL_init()
{
  REPL_MODE = true;
  zuko_src_init(&src);
}
//Implementation
//EXPERIMENTAL!
void REPL()
{
  static string filename;
  k = src.files.size+1;
  filename = "<stdin-"+to_string(k)+">";
  zuko_src_add_file(&src,clone_str(filename.c_str()),clone_str(""));

  
  compiler.reduceStackTo(stackSize);
  lexer lex;
  string line;
  size_t offset = compiler.bytecode.size();
  vector<token> tokens;
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
    string tmp = src.sources.arr[k-1];
    tmp += ((tmp=="") ? "" : "\n") +line;
    src.sources.arr[k-1] = clone_str(tmp.c_str());

    tokens = lexer_generateTokens(&lex,&src,true,k-1);
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
    parser.set_source(&src,k-1);
    ast = parser.parse(tokens,0,tokens.size()-1);

    zobject* constants = new zobject[src.num_of_constants];
    //copy previous constants
    for(int i=0;i<vm.total_constants;i++)
      constants[i] = vm.constants[i];
    if(vm.constants)
      delete[] vm.constants;
    vm.constants = constants;
    compiler.set_source(&src,k-1);
    vector<uint8_t>& bytecode = compiler.compileProgram(ast,0,NULL,Compiler::OPT_COMPILE_DEADCODE); //ask the compiler to add previous bytecode before
    
    stackSize = compiler.globals.size();
    delete_ast(ast);
    
    vm.load(bytecode,&src);
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
    k = src.files.size+1;
    filename = "<stdin-"+to_string(k)+">";
    zuko_src_add_file(&src,clone_str(filename.c_str()),clone_str(""));

  }
}