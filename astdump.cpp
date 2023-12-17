#include <signal.h>
#include <time.h>
#include <cstdint>
#include "include/zuko.h"



int main(int argc, const char* argv[])
{
    if(argc != 2)
      return 0;
    string source_code;
    string filename;
    ProgramInfo p;
    filename = argv[1];
    source_code = readfile(filename);
    p.files.push_back(filename);//add filename
    p.sources.push_back(source_code);//add source

    Lexer lex;
    vector<Token> tokens = lex.generateTokens(filename,source_code);
    if(tokens.size()==0)//empty program nothing to do
      return 0;
    
    Parser parser;
    parser.init(filename,p);//init parser with root filename and ProgramInfo
    Node* ast = parser.parse(tokens); //parse the tokens of root file
    //uncomment below line to print AST in tabular form
    printAST(ast);
    deleteAST(ast);
    
    return 0;
}
