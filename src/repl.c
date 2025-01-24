#include "dyn-str.h"
#include "parser.h"
#include "token.h"
#include "zuko-src.h"
#include "compiler.h"
#include "lexer.h"
#include <stdint.h>
#include <stdio.h>
#include <readline/readline.h>
#include "dis.h"
bool REPL_MODE = false;

zuko_src* src;
size_t stack_size = 0;//total globals added by VM initially
parser_ctx* pctx;
compiler_ctx* cctx;
dyn_str text;
size_t text_size_processed = 0;

void REPL_init()
{
    REPL_MODE = true;
    vm_init();
    dyn_str_init(&text);
    src = create_source("<stdin>",text.arr);
    pctx = create_parser_context(src);
    cctx = create_compiler_ctx(src);
}
//Implementation
//EXPERIMENTAL!
bool is_complete(const char* str)
{
    char ch;
    bool inquotes = false;
    bool escaped = false;
    int i1 = 0;
    int i2 = 0;
    int i3 = 0;
    size_t i = 0;
    while((ch = str[i]))
    {
        if(ch == '"')
        {
            if(!escaped)
                inquotes = !inquotes;
            else
                escaped = false;
        }
        else if(ch == '\\')
        {
            if(escaped)
                escaped = false;
            else
                escaped = true;
        }
        else
        {
            if(escaped)
                escaped = false;
        }
        i++;
    }
}
void repl()
{
    lexer_ctx lex;
    const char* prompt = ">>> ";
    bool continued = false;
    
    while(true)
    {
        if(continued)
            prompt = "... ";
        char* line = readline(prompt);
        if(!line) // EOF
        {
            puts("");
            break;
        }
        dyn_str_append(&text, line);
        dyn_str_push(&text, '\n');
        src->sources.arr[0] = text.arr; // jugaad
        
        token_vector tokens = tokenize(&lex,src,true,0,text_size_processed);
        int32_t i1 = 0;
        int32_t i2 = 0;
        int32_t i3 = 0;
        for(size_t i = 0; i < tokens.size; i++)
        {
            token curr = tokens.arr[i];
            if(curr.type == LParen_TOKEN)
                i1++;
            if(curr.type == L_CURLY_BRACKET_TOKEN)
                i2++;
            if(curr.type == BEGIN_LIST_TOKEN)
                i3++;

            if(curr.type == RParen_TOKEN)
                i1--;
            if(curr.type == R_CURLY_BRACKET_TOKEN)
                i2--;
            if(curr.type == END_LIST_TOKEN)
                i3--;
        }
        if(i1 != 0 || i2!= 0 || i3 != 0)
        {
            token_vector_destroy(&tokens);
            continued = true;
            continue;
        }
        continued = false;
        prompt = ">>> ";

        //parse and execute
        Node* ast = parse_block(pctx,tokens.arr, 0, tokens.size - 1);
        print_ast(ast,0);
        //delete_ast(ast);
        printf("\nexecuting\n");
        uint8_t* bytecode = compile_program(cctx, ast, 0, NULL, OPT_COMPILE_DEADCODE | OPT_NOEXIT);
        dis(bytecode);
        text_size_processed = text.length;
    }
    puts(text.arr);
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
