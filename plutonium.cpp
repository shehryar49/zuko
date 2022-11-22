#define BUILD_FOR_LINUX
#include "include/PltObject.h"
#include "include/plutonium.h"
#include <signal.h>
#include <time.h>
#define PLUTONIUM_VER 1.5
bool REPL_MODE = false;
void signalHandler(int signum)
{
  if(signum==SIGABRT || signum==SIGFPE || signum==SIGILL || signum==SIGSEGV)
  {
    printf("Oops either the interpreter or one of the loaded modules just crashed.Please report this incident.\n");
    exit(0);
  }
}
string readline()
{
  char ch;
  string line;
  while((ch = fgetc(stdin))!='\n')
  {
    line+=ch;
  }
  return line;
}

string& readfile(string filename)
{
  FILE* fp = fopen(filename.c_str(), "r");
  static string src;
  src = "";
  if (!fp)
  {
      printf("Error opening file: %s\n", strerror(errno));
      exit(0);
  }

  signed char ch;
  while ((ch = fgetc(fp)) != EOF)
  {
      if(ch <= 0 || ch > 127)
      {
        printf("Error the file %s does not seem to be a text file or contains non-ascii characters.\n",filename.c_str());
        exit(0);
      }
      src += ch;
  }
  fclose(fp);
  return src;
}
void WriteByteCode(vector<unsigned char>& bytecode,std::unordered_map<size_t,ByteSrc>& LineNumberTable,vector<string>& files)
{

   FILE* f = fopen("program.pltb","wb");
   if(!f)
   {
    printf("error writing bytecode file \n");
    return;
   }
    //write line number table
    int total = LineNumberTable.size();
    fwrite(&total,sizeof(int),1,f);

    for(auto e: LineNumberTable)
    {
      size_t Pos = e.first;
      ByteSrc src = e.second;
      fwrite(&Pos,sizeof(size_t),1,f);
      Pos = src.ln;
      fwrite(&Pos,sizeof(size_t),1,f);
      string s = files[src.fileIndex];
      int L  = s.length();
      fwrite(&L,sizeof(int),1,f);
      fwrite(s.c_str(),sizeof(char),s.length(),f);
    }
    //
   unsigned char arr[bytecode.size()];
   total = vm.total_constants;
   fwrite(&total,sizeof(int),1,f);
   for(int i=0;i<vm.total_constants;i+=1)
   {
     string s = PltObjectToStr(vm.constants[i]);
    // printf("writing constant %s\n",s.c_str());
     int L = s.length();
     fwrite(&L,sizeof(int),1,f);
     fwrite(s.c_str(),sizeof(char),s.length(),f);
   }
   int sz = bytecode.size();
   fwrite(&sz,sizeof(int),1,f);
    for(int k=0;k<bytecode.size();k+=1)
    {
      arr[k] = bytecode[k];
    }
    fwrite(arr,sizeof(unsigned char),bytecode.size(),f);
    fclose(f);
}

void REPL()
{
  REPL_MODE = true;
  static vector<string> sources;
  static vector<string> files;
  static string filename;
  static int k = 0;
  k = files.size()+1;
  filename = "<stdin-"+to_string(k)+">";
  files.push_back(filename);
  sources.push_back("");
  static std::unordered_map<size_t,ByteSrc> LineNumberTable;
  vector<string> fnReferenced;
  Lexer lex;
  string line;
  static std::vector<unsigned char> bytecode;
  size_t offset = bytecode.size();
  static int noc = 1;
  vector<Token> tokens;
  Node* ast;
  static Parser parser;
  static Compiler compiler;
  initFunctions();
  initMethods();
  bool continued = false;
  while(true)
  {
    if(!continued)
      printf(">>> ");
    else
      printf("... ");
    continued=false;
    line = readline();
    if(line=="exit" || line=="quit" || line=="baskardebhai" || line=="yawr")
      break;
    if(line=="")
    {
      continue;
    }
    sources[k-1] += ((sources[k-1]=="") ? "" : "\n") +line;
    tokens = lex.generateTokens(filename,sources[k-1]);
    int i1=0,i2=0,i3=0;
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
    parser.init(&fnReferenced,&noc,&files,&sources,filename);
    ast = parser.parse(tokens);
    PltObject* constants = new PltObject[noc];
    for(int i=0;i<vm.total_constants;i++)
      constants[i] = vm.constants[i];
    if(vm.constants)
      delete[] vm.constants;
    vm.constants = constants;
    compiler.init(&fnReferenced,&noc,&files,&sources,&LineNumberTable,filename);
    bytecode = compiler.compileProgram(ast,0,NULL,bytecode,true,false); //ask the compiler to add previous bytecode before
    deleteAST(ast);
    //new bytecode
    //how much is the retarded user gonna write anyway in REPL
    
    vm.load(bytecode,&LineNumberTable,&files,&sources);
    //WriteByteCode(bytecode,LineNumberTable,files);// for debugging
    vm.interpret(offset,false);
    bytecode.pop_back();
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
      vm.constants = new PltObject[1];
      vm.total_constants = 1;
      printf("Plutonium Programming Langauge v%.2f build date(%s %s)\nCreated by Shahryar Ahmad\nREPL Mode(Experimental)\n", PLUTONIUM_VER,__DATE__,__TIME__);
      REPL();
      return 0;
    }
    string source_code;
    string filename;
    std::unordered_map<size_t, ByteSrc> LineNumberTable;
    vector<string> files;// filenames
    vector<string> sources;
    if(argc>=3 && strcmp(argv[1],"-c")==0)
    {
      filename = "argv";
      source_code = argv[2];
    }
    else
    {
      filename = argv[1];
      source_code = readfile(filename);
    }

    vector<unsigned char> bytecode;
    files.push_back(filename);//add to the list of compiled files
    sources.push_back(source_code);
    {
        int num_of_constants = 1;
        vector<string> fnReferenced;//stores the names of functions that are actually called in the program
        //functions that are not being called will not be compiled
        Lexer lex;
        vector<Token> tokens = lex.generateTokens(filename,source_code);
        if(tokens.size()==0)//empty program nothing to do
          return 0;
        Parser parser;
        //the parser handles file imports so address of two  string vectors are passed
        //initially files vector has only 1 name i.e the main file but parser can add to it
        //the sources vector also has initially 1 element but the parser adds to it as it reads more files
        //the names of functions that are referenced in plutonium code are place in fnReferenced
        //vector by the parser
        parser.init(&fnReferenced,&num_of_constants,&files,&sources,filename);
        Node* ast = parser.parse(tokens);
    
      //  printAST(ast);
      //  return 0;
        Compiler compiler;
        compiler.init(&fnReferenced,&num_of_constants,&files,&sources,&LineNumberTable,filename);
        vm.constants = new PltObject[num_of_constants];
        vm.total_constants = 1;
        initFunctions();
        initMethods();
        bytecode = compiler.compileProgram(ast,argc,argv);
        deleteAST(ast);
        tokens.clear();

    }

  
    vm.load(bytecode,&LineNumberTable,&files,&sources);
    bytecode.clear();
    bytecode.shrink_to_fit();
    vm.interpret();


    return 0;
}
