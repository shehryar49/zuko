#ifndef PLT_REPL_H_
#define PLT_REPL_H_
#include "plutonium.h"
using namespace std;


// REPL STATE
bool REPL_MODE = false;
static ProgramInfo p;
static int32_t k = 0;
static size_t stackSize = 19;//total globals added by VM initially
static Compiler compiler;
static Parser parser;

//Implementation
//EXPERIMENTAL!
void REPL()
{
  REPL_MODE = true;
  static string filename;
  k = p.files.size()+1;
  filename = "<stdin-"+to_string(k)+">";
  p.files.push_back(filename);
  p.sources.push_back("");
  
  compiler.reduceStackTo(stackSize);
  Lexer lex;
  string line;
  size_t offset = compiler.bytecode.size();
  vector<Token> tokens;
  Node* ast;
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
    p.sources[k-1] += ((p.sources[k-1]=="") ? "" : "\n") +line;
    tokens = lex.generateTokens(filename,p.sources[k-1]);
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
    parser.init(filename,p);

    ast = parser.parse(tokens);
    PltObject* constants = new PltObject[p.num_of_constants];
    //copy previous constants
    for(int i=0;i<vm.total_constants;i++)
      constants[i] = vm.constants[i];
    if(vm.constants)
      delete[] vm.constants;
    vm.constants = constants;
    compiler.init(filename,p);
    vector<uint8_t>& bytecode = compiler.compileProgram(ast,0,NULL,true,false); //ask the compiler to add previous bytecode before
    
    stackSize = compiler.globals.size();
    deleteAST(ast);
    
    vm.load(bytecode,p);
    //WriteByteCode(bytecode,LineNumberTable,files);// for debugging
    vm.interpret(offset,false);//
    if(vm.STACK.size() > stackSize)
        vm.STACK.erase(vm.STACK.begin()+stackSize,vm.STACK.end());
    else if(vm.STACK.size() < stackSize)
    {
      compiler.reduceStackTo(vm.STACK.size());
      stackSize = vm.STACK.size();
    }
    bytecode.pop_back();//pop OP_EXIT
    offset=bytecode.size();
    k = p.files.size()+1;
    filename = "<stdin-"+to_string(k)+">";
    p.files.push_back(filename);
    p.sources.push_back("");
  }
}
#endif