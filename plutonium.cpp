#define BUILD_FOR_LINUX

#include <time.h>
#include "include/PltObject.h"
#include "include/plutonium.h"
#include <csignal>
size_t line_num = 1;
string source_code;
string filename;
std::unordered_map<size_t, ByteSrc> LineNumberTable;
vector<string> files;
vector<string> sources;
short fileTOP = 0;
VM vm;
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

string readfile(string filename)
{
  FILE* fp = fopen(filename.c_str(), "r");
  string src;
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

#define PLUTONIUM_VER 1.5
int main(int argc, char** argv)
{
    signal(SIGFPE,signalHandler);
    signal(SIGILL,signalHandler);
    signal(SIGABRT,signalHandler);
    signal(SIGSEGV,signalHandler);
    if (argc < 2)
    {
        printf("Plutonium Programming Langauge v%.2f\nCreated by Shahryar Ahmad\n", PLUTONIUM_VER);
        printf("usage: plutonium <filename>\n");
        return 0;
    }
    filename = argv[1];
    vector<unsigned char> bytecode;
    files.push_back(filename);//add to the list of compiled files
    fileTOP = 0;//current file at index 0
    source_code = readfile(filename);
    sources.push_back(source_code);
    {
        int num_of_constants = 1;
        vector<string> fnReferenced;//stores the names of functions that are actually called in the program
        //functions that are not being called will not be compiled
        vector<Token> tokens = generateTokens(source_code);
        if(tokens.size()==0)//empty program nothing to do
          return 0;
        Parser parser;
        parser.init(&fnReferenced,&num_of_constants);
        Node* ast = parser.parse(tokens);
      //  printAST(ast);
      //  return 0;
        Compiler compiler;
        compiler.init(3,&vm,&fnReferenced,&num_of_constants);
        compiler.globals.emplace("argv",0);
        compiler.globals.emplace("stdin",1);
        compiler.globals.emplace("stdout",2);
        vm.constants = new PltObject[num_of_constants];
        vm.total_constants = 1;
        initFunctions();
        initMethods();
        bytecode = compiler.compileProgram(ast);
        deleteAST(ast);
        tokens.clear();
        
    }

    //Code for writing the bytecode to a file

   /*FILE* f = fopen("program.pltb","wb");
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
*/
    //return 0;
    int k = 2;
    PltList l;
    PltObject elem;
    elem.type = 's';
    while (k < argc)
    {
        elem.s = argv[k];
        l.push_back(elem);
        k += 1;
    }
    PltObject A;
    A.type = 'j';
    PltList* p = vm.allocList();
    *p = l;
    A.ptr = (void*)p;
    vm.STACK.push_back(A);
    FileObject* STDIN = vm.allocFileObject();
    STDIN->open = true;
    STDIN ->fp = stdin;

    FileObject* STDOUT = vm.allocFileObject();
    STDOUT->open = true;
    STDOUT ->fp = stdout;
    A.type = 'u';
    A.ptr = (void*)STDIN;
    vm.STACK.push_back(A);
    A.type = 'u';
    A.ptr = (void*)STDOUT;
    vm.STACK.push_back(A);
    vm.load(bytecode);
    vm.interpret();
    delete[] vm.constants;
    return 0;
}
