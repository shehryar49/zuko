#ifndef PLT_PROGINFO_H_
#define PLT_PROGINFO_H_
#include <vector>
#include <string>
#include <unordered_map>
#include <cstdint>
using namespace std;
struct ByteSrc //struct to store source of a bytecode index
{
  short fileIndex; // index of filename in files vector
  size_t ln;//line number of that file
};

//Data structure to represent zuko source code and some other metadata
class ZukoSource
{
public:
  int32_t num_of_constants = 0;//number of constants in program (set by compiler and this is also the length of vm.constants )
  std::unordered_map<string,vector<string>> refGraph;//refGraph of functions and classes
  //generated by the parser and used by the compiler to eliminate deadcode
  vector<string> files; //filenames
  vector<string> sources; //source codes
  std::unordered_map<size_t, ByteSrc> LineNumberTable;//line number table (maps a bytecode index to some line number in a source file)
  void addFile(const std::string& filename,const std::string& src)
  {
    files.push_back(filename);
    sources.push_back(src);
  }
  void reset()
  {
    num_of_constants = 0;
    refGraph.clear();
    files.clear();
    sources.clear();
    LineNumberTable.clear();
  }
};

#endif