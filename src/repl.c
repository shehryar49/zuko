#include "parser.h"
#include "zuko-src.h"
#include "compiler.h"
#include "lexer.h"
#include <stdint.h>
#include <stdio.h>
#include <readline/readline.h>

bool REPL_MODE = false;
zuko_src* src;
int32_t k = 0;
size_t stack_size = 0;//total globals added by VM initially
parser_ctx* pctx;
compiler_ctx* cctx;
void REPL_init()
{
    REPL_MODE = true;
    src = create_empty_source();
    //cctx = create_compiler_ctx(src);
  //zuko_src_init(&src);
  //parser_init(&parser);
}
//Implementation
//EXPERIMENTAL!

void repl()
{
    char filename[256];
    k = src->files.size + 1;
    snprintf(filename,256,"<stdin-%d>",k);
    //compiler_reduceStackTo(cctx, stack_size);
    lexer_ctx lex;
    //size_t offset = cctx->bytes_done;
    token_vector tokens;
    const char* prompt = ">>> ";
    bool continued;
    while(true)
    {
        if(continued)
            prompt = "... ";
        char* line = readline(prompt);
        if(!line) // EOF
        {
            puts("");
            exit(0);
        }
        zuko_src_add_file(src,filename,line);
        tokens = tokenize(&lex,src,true,k);
    }
}
void REPL()
{
/*  char filename[256];
  k = src->files.size+1;
  snprintf(filename,256,"<stdin-%d>",k);
  zuko_src_add_file(&src,clone_str(filename),clone_str(""));

  
  compiler.reduceStackTo(stackSize);
  lexer lex;
  string line;
  size_t offset = compiler.bytecode.size;
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
      for(size_t i=0;i<STACK.size;i++)
      {
        zobject e = STACK.arr[i];
        printf("%s\n",zobjectToStr(e).c_str());
      }
      continue;
    }
    else if(line == ".showconstants")
    {
      printf("vm.total_constants = %d\n",vm.total_constants);
      for(int i=0;i<vm.total_constants;i++)
      {
        zobject e = vm_constants[i];
        printf("%s\n",zobjectToStr(e).c_str());
      }
      continue;
    }
    else if(line=="")
      continue;
    string tmp = src.sources.arr[k-1];
    tmp += ((tmp=="") ? "" : "\n") +line;
    src.sources.arr[k-1] = clone_str(tmp.c_str());

    token_vector t = lexer_generateTokens(&lex,&src,true,k-1);
    tokens.clear();
    vector<token> tokens;
    for(size_t i=0;i<t.size;i++)
        tokens.push_back(t.arr[i]);
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
    parser_set_source(&parser,&src,k-1);
    ast = parse_block(&parser,&tokens[0],0,tokens.size()-1);

    zobject* constants = new zobject[src.num_of_constants];
    //copy previous constants
    for(int i=0;i<vm.total_constants;i++)
      constants[i] = vm_constants[i];
    if(vm_constants)
      delete[] vm_constants;
    vm_constants = constants;
    compiler.set_source(&src,k-1);
    uint8_t* bytecode = compiler.compileProgram(ast,0,NULL,Compiler::OPT_COMPILE_DEADCODE); //ask the compiler to add previous bytecode before
    
    stackSize = compiler.globals.size;
    delete_ast(ast);
    
    vm.load(bytecode,&src);
    vm.interpret(offset,false);//
    if(STACK.size > stackSize)
        zlist_erase_range(&STACK,stackSize,STACK.size - 1);
    else if(STACK.size < stackSize)
    {
      compiler.reduceStackTo(STACK.size);
      stackSize = STACK.size;
    }
//    bytecode.pop_back();//pop OP_EXIT
  //  offset=bytecode.size();
    k = src.files.size+1;
    filename = "<stdin-"+to_string(k)+">";
    zuko_src_add_file(&src,clone_str(filename.c_str()),clone_str(""));

  }*/
}
