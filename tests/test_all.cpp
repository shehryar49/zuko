#include <iostream>
#include <fstream>
#include <string>
#define red "\e[0;31m"
#define green "\e[0;32m"
#define reset "\e[0m"
using namespace std;
const char* tests[] = 
{
    "helloworld",
    "variables",
    "if",
    "ifelse",
    "ifelseif",
    "ifelseifelse",
    "while",
    "dowhile",
    "for",
    //"lists", uses some features not available in current stable release
    "dicts",
    "byte",
    "bytearray",
    "types",
    "foreach",
    "functions",
    "defaultArgs",
    "classes",
    "operatorOverloading",
    "extendedClasses",
    "namespaces",
    "scopes",
    "dmas",
    "misc",
    "factorial",
    "gcd",
    "quicksort",
    "mergesort",
    "insertionsort",
    "selectionsort",
    "bubblesort",
    "bsearch",
    "permutations",
    "huffman",
    "pyramid",
    "awkwardLexer",
    "polynomial"
};
int main(int argc,const char* argv[])
{
  string exePath;
  if(argc == 1)
  {
    #ifdef _WIN32
    exePath = "..\\plutonium ";
    #else
      exePath = "../plutonium ";
    #endif
  }
  else
  {
    exePath = argv[1];
    exePath+=" ";
  }
  int n = sizeof(tests)/sizeof(const char*);
  int failed = 0;
  string expectedOutput;
  string output;
  string test;
  string line;

  cout<<"Using binary "<<exePath<<endl;
  for(int i=1;i<=n;i++)
  {
    output = "";
    expectedOutput = "";
    line = "";
    test = tests[i-1];
    ifstream fin("outputs/"+test+".txt",ios::in);
    if(!fin)
    {
   //   cout<<"Output file outputs/"<<test<<".txt does not exist!"<<endl;
      cout<<"Test "<<test<<" (failed)"<<endl;
      failed+=1;
      continue;
    }
    char ch;
    while(fin.get(ch))
      expectedOutput+= ch;
    fin.close();
    line = exePath + test+".plt > out.txt";
    int l = system(line.c_str());//produces out.txt
    ifstream file("out.txt",ios::in);
    if(l!=0 || !file)
    {
      cout<<"Test "<<test<<" (failed)"<<endl;
      failed+=1;
      continue;
    }
    while(file.get(ch))
      output+= ch;
    file.close();

    if(output == expectedOutput)
      cout<<"Test "<<test<<green<<" (success)"<<reset<<endl;
    else
    {
     cout<<"Test "<<test<<red<<" (failed)"<<reset<<endl;
     failed+=1;
    }
  }
  cout<<endl;
  cout<<(n-failed)<<"/"<<n<<" tests executed successfully!"<<endl;
  cout<<failed<<"/"<<n<<" tests failed."<<endl;
  if(failed != 0)
    return 1;
  return 0;
}