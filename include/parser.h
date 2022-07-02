#ifndef PARSER_H_
#define PARSER_H_
#include "plutonium.h"
#include "token.h"
#include "lexer.h"
#include <algorithm>
using namespace std;
extern string filename;
extern vector<string> sources;
extern vector<string> files;
extern short fileTOP;
//so that we only compile the needed functions :)

struct Node
{
  string val;
  vector<Node*> childs;
};
int len(string s)
{
  return s.length()-1;
}
string substr(int  x,int  y,string s)
{
  //Crash safe substr function, the std::string.substr() can cause program to crash
    int  k = x;
	string p = "";
	while(k<=y && k<s.length())
	{
		p+= s[k];
		k+=1;
	}
	return p;
}
vector<string> split(string s,string x)
{
	unsigned int  k = 0;
	vector<string> list;
	while(s.find(x)!=std::string::npos)
	{
		k = s.find(x);
		list.push_back(s.substr(0,k));
		s = s.substr(k+x.length());
	}
	list.push_back(s);
	return list;
}
string lstrip(string s)
{
    while((s.length()>0) && (s[0]==' ' || s[0]=='\t'))
    {
        s = s.substr(1);
    }
    return s;
}
string replace(string x,string y,string s)//Replaces only once
{
	size_t start = s.find(x);
	if(start!=std::string::npos)
	{
		string p1 = substr(0,start-1,s);
		string p2 = substr(start+len(x)+1,len(s),s);
		string result = p1+y+p2;
		return result;
	}
	return s;
}
Node* NewNode(string val="")
{
  Node* p = new(nothrow) Node;
  if(!p)
  {
    printf("error allocating memory!\n");
    exit(0);
  }
  p->val = val;
  return p;
}
void removeUselessNewlines(vector<Token>& tokens)
{
    if(tokens.size()==0)
        return;
    while(tokens[0].type==NEWLINE_TOKEN && tokens.size()>0)
        tokens.erase(tokens.begin());
    if(tokens.size()==0)
        return;
    while(tokens[tokens.size()-1].type==NEWLINE_TOKEN && tokens.size()>0)
        tokens.pop_back();
}
int matchRPRight(int k,const vector<Token>& tokens)
{
  int ignore = 0;
  while(k>=0)
  {
    if(tokens[k].type== TokenType::RParen_TOKEN)
     ignore+=1;
    else if(tokens[k].type== TokenType::LParen_TOKEN)
    {
      ignore-=1;
      if(ignore==0)
        return k;
    }
    k-=1;
  }
  return -1;
}
int findBeginList(int k,const vector<Token>& tokens)
{
  int ignore = 0;
  while(k>=0)
  {
    if(tokens[k].type==TokenType::END_LIST_TOKEN)
     ignore+=1;
    else if(tokens[k].type== TokenType::BEGIN_LIST_TOKEN)
    {
      ignore-=1;
      if(ignore==0)
        return k;
    }
    k-=1;
  }
  return -1;
}
int findLCBRight(int k,const vector<Token>& tokens)
{
  int ignore = 0;
  while(k>=0)
  {
    if(tokens[k].type== TokenType::R_CURLY_BRACKET_TOKEN)
     ignore+=1;
    else if(tokens[k].type== TokenType::L_CURLY_BRACKET_TOKEN)
    {
      ignore-=1;
      if(ignore==0)
        return k;
    }
    k-=1;
  }
  return -1;
}
int findToken(Token t,int start,const vector<Token>& tokens)
{
   while(start<tokens.size())
   {
     if(tokens[start].type==t.type && tokens[start].content==t.content)
       return start;
     start+=1;
   }
   return -1;
}
int matchRP(int k,const vector<Token> tokens)
{
  int ignore = 0;
  while(k<tokens.size())
  {
    if(tokens[k].type== TokenType::LParen_TOKEN)
      ignore+=1;
    else if(tokens[k].type== TokenType::RParen_TOKEN)
    {
      ignore-=1;
      if(ignore==0)
        return k;
    }
    k+=1;
  }
  return -1;
}
int findEndList(int k,const vector<Token>& tokens)
{
  int ignore = 0;
  while(k<tokens.size())
  {
    if(tokens[k].type== TokenType::BEGIN_LIST_TOKEN)
      ignore+=1;
    else if(tokens[k].type== TokenType::END_LIST_TOKEN)
    {
      ignore-=1;
      if(ignore==0)
        return k;
    }
    k+=1;
  }
  return -1;
}
int findRCB(int k,const vector<Token>& tokens)
{
  int ignore = 0;
  while(k<tokens.size())
  {
    if(tokens[k].type== TokenType::L_CURLY_BRACKET_TOKEN)
      ignore+=1;
    else if(tokens[k].type== TokenType::R_CURLY_BRACKET_TOKEN)
    {
      ignore-=1;
      if(ignore==0)
        return k;
    }
    k+=1;
  }
  return -1;
}

void deleteAST(Node* ast)
{
  if(!ast)
    return;
  for(size_t k = 0;k<ast->childs.size();k+=1)
  {
     deleteAST(ast->childs[k]);
  }
  delete ast;
}
void CopyAst(Node*& dest,Node* src)
{
    //Copies one tree structure value by value
    if(!src)
        return;
    dest->val = src->val;
    int k = 0;
    for(k=0;k<src->childs.size();k+=1)
    {
        Node* n = new Node;
        CopyAst(n,src->childs[k]);
        dest->childs.push_back(n);
    }
}
const char* StrTokenTypes[] =
{
  "id",
  "string",
  "num",
  "float",
  "keyword",
  "comma",
  "colon",
  "L_CURLY_BRACKET_TOKEN",// {
  "R_CURLY_BRACKET_TOKEN",// }
  "LParen_TOKEN",// (
  "RParen_TOKEN",// )
  "BEGIN_LIST_TOKEN",// [
  "END_LIST_TOKEN",// ]
  "op",
  "newline",
  "byte",
  "bool",
  "EOP",
  "Ellipsis"
};

// Function to print AST in tablular form
void printAST(Node* n,int spaces = 0)
{
   if(!n)
   {

     return;
   }
   printf("|%s\n",n->val.c_str());
   spaces+=1+1;
   for(size_t k=0;k<(n->childs.size());k+=1)
   {
       for(int j=0;j<spaces;j++)
       {
           printf(" ");
       }
       printAST(n->childs[k],spaces);
   }
}
int findTokenConsecutive(Token t,int start,const vector<Token>& tokens)
{
    for(int k=start;k<tokens.size();k+=1)
    {
        if(tokens[k].type==t.type && tokens[k].content==t.content)
        {
            return k;
        }
        else if(tokens[k].type== TokenType::NEWLINE_TOKEN)
        {

        }
        else
        {

            return -1;
        }
    }
    return -1;
}
class Parser
{
private:
  vector<Token> known_constants;
  vector<string> prefixes = {""};//for namespaces
  vector<string>* fnReferenced;
  //block of each udf function
  size_t line_num = 1;
  int* num_of_constants;
  bool foundYield = false;
public:
  std::unordered_map<string,vector<string>> fntable;//to store list of functions referenced in
  void init(vector<string>* fnr,int* p)
  {
    fnReferenced = fnr;
    num_of_constants = p;
  }
  void parseError(string type,string msg,bool dummy)
  {
    printf("\nFile %s\n",filename.c_str());
    printf("%s at line %ld\n",type.c_str(),line_num);
    auto it = std::find(files.begin(),files.end(),filename);
    size_t i = it-files.begin();
    string source_code = sources[i];
    int l = 1;
    string line = "";
    int k = 0;
    while(l<=line_num)
    {

        if(source_code[k]=='\n')
        {
            l+=1;
        }
        else if(l==line_num)
            line+=source_code[k];
        k+=1;
        if(k>=source_code.length())
        {
          break;
        }
    }
    printf("%s\n",lstrip(line).c_str());
    printf("%s\n",msg.c_str());
    exit(0);
  }
  void addToFnReferenced(string name)
  {
  //    printf("trying to add %s to fnr\n",name.c_str());
    if(std::find(fnReferenced->begin(),fnReferenced->end(),name)==fnReferenced->end())
    {
      if(fntable.find(name)!=fntable.end())
      {
        fnReferenced->push_back(name);
        vector<string> dependencies = fntable[name];
        for(auto e: dependencies)
        {
          addToFnReferenced(e);
        }
      }

    }
  }
  Node* parseExpr(vector<Token> tokens)
  {

    if(tokens.size()==0)
      parseError("SyntaxError","Invalid Syntax",false);
    //Tokens.size is never zero
    Node* ast = nullptr;
    
    if(tokens.size()==1)
    {
       if(tokens[0].type== TokenType::KEYWORD_TOKEN && tokens[0].content=="nil" )
       {
         ast = NewNode((string)StrTokenTypes[tokens[0].type]+": "+tokens[0].content);
         return ast;
       }
       if(tokens[0].type== TokenType::BYTE_TOKEN || tokens[0].type== TokenType::FLOAT_TOKEN || tokens[0].type== TokenType::BOOL_TOKEN || tokens[0].type== TokenType::NUM_TOKEN || tokens[0].type== TokenType::STRING_TOKEN || tokens[0].type== TokenType::ID_TOKEN)
       {
         if(tokens[0].type!=ID_TOKEN && findToken(tokens[0],0,known_constants)==-1)
         {
           known_constants.push_back(tokens[0]);
           *num_of_constants = *num_of_constants + 1;
         }
         if(tokens[0].type==ID_TOKEN && fntable.find(tokens[0].content)!=fntable.end())
         {
           addToFnReferenced(tokens[0].content);
           if(fntable.find("@"+tokens[0].content)!=fntable.end())
             addToFnReferenced("@"+tokens[0].content);
           //the user might be referring to function in global namespace or in current namespace
           //so we add both names to fnReferenced

           if(prefixes.size()>1)
             addToFnReferenced(prefixes.back()+tokens[0].content);
         }
         ast = NewNode((string)StrTokenTypes[tokens[0].type]+": "+tokens[0].content);
         return ast;
       }
       parseError("SyntaxError","Invalid Syntax",false);
    }
    
    
    /////////
    int k = tokens.size()-1;
    while(k>=0)
    {
      if(tokens[k].type== TokenType::RParen_TOKEN)
      {
        int i = matchRPRight(k,tokens);
        if(i==-1)
          parseError("SyntaxError","Invalid Syntax",false);
        k = i;
      }
      else if(tokens[k].type== TokenType::END_LIST_TOKEN)
      {
         int i = findBeginList(k,tokens);
         if(i==-1)
           parseError("SyntaxError","Invalid Syntax",false);
         k=i;
      }
      else if(tokens[k].type== TokenType::R_CURLY_BRACKET_TOKEN)
      {
        int i = findLCBRight(k,tokens);
        if(i==-1)
          parseError("SyntaxError","Invalid Syntax",false);
        k=i;
      }
      else if(tokens[k].type== TokenType::OP_TOKEN && (tokens[k].content=="and" || tokens[k].content=="or" || tokens[k].content=="is"))
      {
        vector<Token> lhs = {tokens.begin(),tokens.begin()+k};
        vector<Token> rhs = {tokens.begin()+k+1,tokens.end()};
        if(lhs.size()==0 || rhs.size()==0)
          parseError("SyntaxError","Invalid Syntax",false);
        Node* ast = NewNode(tokens[k].content);
        ast->childs.push_back(parseExpr(lhs));
        ast->childs.push_back(parseExpr(rhs));
        return ast;
      }
      k-=1;
    }
    //////
    ////
    //////////
    k = tokens.size()-1;
    while(k>=0)
    {
      if(tokens[k].type== TokenType::RParen_TOKEN)
      {
        int i = matchRPRight(k,tokens);
        if(i==-1)
          parseError("SyntaxError","Invalid Syntax",false);
        k = i;
      }
      else if(tokens[k].type== TokenType::END_LIST_TOKEN)
      {
         int i = findBeginList(k,tokens);
         if(i==-1)
           parseError("SyntaxError","Invalid Syntax",false);
         k=i;
      }
      else if(tokens[k].type== TokenType::R_CURLY_BRACKET_TOKEN)
      {
        int i = findLCBRight(k,tokens);
        if(i==-1)
          parseError("SyntaxError","Invalid Syntax",false);
        k=i;
      }
      else if(tokens[k].type== TokenType::OP_TOKEN && (tokens[k].content=="==" || tokens[k].content=="!=" || tokens[k].content=="<" || tokens[k].content==">" || tokens[k].content=="<=" || tokens[k].content==">="))
      {
        vector<Token> lhs = {tokens.begin(),tokens.begin()+k};
        vector<Token> rhs = {tokens.begin()+k+1,tokens.end()};
        if(lhs.size()==0 || rhs.size()==0)
          parseError("SyntaxError","Invalid Syntax",false);
        Node* ast = NewNode(tokens[k].content);
        ast->childs.push_back(parseExpr(lhs));
        ast->childs.push_back(parseExpr(rhs));
        return ast;
      }
      k-=1;
    }
    /////
    k = tokens.size()-1;
    while(k>=0)
    {
      if(tokens[k].type==TokenType::RParen_TOKEN)
      {
        int i = matchRPRight(k,tokens);
        if(i==-1)
          parseError("SyntaxError","Invalid Syntax",false);
        k = i;
      }
      else if(tokens[k].type== TokenType::END_LIST_TOKEN)
      {
         int i = findBeginList(k,tokens);
         if(i==-1)
           parseError("SyntaxError","Invalid Syntax",false);
         k=i;
      }
      else if(tokens[k].type== TokenType::R_CURLY_BRACKET_TOKEN)
      {
        int i = findLCBRight(k,tokens);
        if(i==-1)
          parseError("SyntaxError","Invalid Syntax",false);
        k=i;
      }
      else if(tokens[k].type== TokenType::OP_TOKEN && (tokens[k].content=="<<" || tokens[k].content==">>" || tokens[k].content=="&" || tokens[k].content=="|" || tokens[k].content=="^"))
      {
        vector<Token> lhs = {tokens.begin(),tokens.begin()+k};
        vector<Token> rhs = {tokens.begin()+k+1,tokens.end()};
        if(lhs.size()==0 || rhs.size()==0)
          parseError("SyntaxError","Invalid Syntax",false);
        Node* ast = NewNode(tokens[k].content);
        ast->childs.push_back(parseExpr(lhs));
        ast->childs.push_back(parseExpr(rhs));
        return ast;
      }
      k-=1;
    }
    //////////
    k = tokens.size()-1;
    while(k>=0)
    {
      if(tokens[k].type== TokenType::RParen_TOKEN)
      {
        int i = matchRPRight(k,tokens);
        if(i==-1)
          parseError("SyntaxError","Invalid Syntax",false);
        k = i;
      }
      else if(tokens[k].type== TokenType::END_LIST_TOKEN)
      {
         int i = findBeginList(k,tokens);
         if(i==-1)
           parseError("SyntaxError","Invalid Syntax",false);
         k=i;
      }
      else if(tokens[k].type== TokenType::R_CURLY_BRACKET_TOKEN)
      {
        int i = findLCBRight(k,tokens);
        if(i==-1)
          parseError("SyntaxError","Invalid Syntax",false);
        k=i;
      }
      else if(tokens[k].type==OP_TOKEN && (tokens[k].content=="+" || tokens[k].content=="-") && k!=0)
      {
        vector<Token> lhs = {tokens.begin(),tokens.begin()+k};
        vector<Token> rhs = {tokens.begin()+k+1,tokens.end()};
        if(lhs.size()==0 || rhs.size()==0 )
          parseError("SyntaxError","Invalid Syntax",false);
        Node* ast = NewNode(tokens[k].content);
        ast->childs.push_back(parseExpr(lhs));
        ast->childs.push_back(parseExpr(rhs));
        return ast;
      }
      k-=1;
    }

    k = tokens.size()-1;
    while(k>=0)
    {
       if(tokens[k].type== TokenType::RParen_TOKEN)
       {
         int i = matchRPRight(k,tokens);
         if(i==-1)
          parseError("SyntaxError","Invalid Syntax",false);
         k = i;
       }
       else if(tokens[k].type== TokenType::END_LIST_TOKEN)
       {
          int i = findBeginList(k,tokens);
          if(i==-1)
           parseError("SyntaxError","Invalid Syntax",false);
          k=i;
       }
       else if(tokens[k].type== TokenType::R_CURLY_BRACKET_TOKEN)
      {
        int i = findLCBRight(k,tokens);
        if(i==-1)
          parseError("SyntaxError","Invalid Syntax",false);
        k=i;
      }
       else if(tokens[k].type== TokenType::OP_TOKEN && (tokens[k].content=="/" || tokens[k].content=="*" || tokens[k].content=="%"))
       {
         vector<Token> lhs = {tokens.begin(),tokens.begin()+k};
         vector<Token> rhs = {tokens.begin()+k+1,tokens.end()};
         if(lhs.size()==0 || rhs.size()==0)
          parseError("SyntaxError","Invalid Syntax",false);
         Node* ast = NewNode(tokens[k].content);
         ast->childs.push_back(parseExpr(lhs));
         ast->childs.push_back(parseExpr(rhs));
         return ast;
       }
      k-=1;
     }
     /////////////////

     ////////////////
    if(tokens[0].type== TokenType::OP_TOKEN)
    {

      if(tokens[0].content=="-")
      {
        Node* ast = NewNode("neg");
        vector<Token> e = {tokens.begin()+1,tokens.end()};
        if(e.size()==0)
          parseError("SyntaxError","Invalid Syntax",false);
        ast->childs.push_back(parseExpr(e));
        return ast;
      }
      if(tokens[0].content=="+")
      {
        vector<Token> e = {tokens.begin()+1,tokens.end()};
        if(e.size()==0)
          parseError("SyntaxError","Invalid Syntax",false);
        return (parseExpr(e));
      }
      if(tokens[0].content=="~")
      {
        Node* ast = NewNode("~");
        vector<Token> e = {tokens.begin()+1,tokens.end()};
        if(e.size()==0)
          parseError("SyntaxError","Invalid Syntax",false);
        ast->childs.push_back(parseExpr(e));
        return ast;
      }

    }
    if(tokens[0].type== TokenType::OP_TOKEN && tokens[0].content=="!")
    {
      Node* ast = NewNode("!");
      vector<Token> e = {tokens.begin()+1,tokens.end()};
      ast->childs.push_back(parseExpr(e));
      return ast;
    }
    if(tokens[tokens.size()-1].type==END_LIST_TOKEN)// evaluate indexes
    {
       int i = findBeginList(tokens.size()-1,tokens);
       if(i==-1)
         parseError("SyntaxError","Invalid Syntax",false);
       if(i!=0)
       {
          vector<Token> toindex = {tokens.begin(),tokens.begin()+i};
          vector<Token> index = {tokens.begin()+i+1,tokens.end()-1};
          //printf("handled here\n");
        //  printf("toindex = ");
      //    for(auto e: toindex)
        //    printf("%s",e.content.c_str());
      //    printf("\n");

          Node* ast = NewNode("index");
          ast->childs.push_back(parseExpr(toindex));
          ast->childs.push_back(parseExpr(index));
          return ast;
       }
       else
       {
         ast = NewNode("list");
         if(tokens.size()==2)
           return ast;
         vector<Token> t;
         for(int k = 1;k<=tokens.size()-2;k+=1)
         {
           if(tokens[k].type== TokenType::COMMA_TOKEN)
           {
             if(t.size()==0)
               parseError("SyntaxError","Error expected an element before ',' ",false);
             ast->childs.push_back(parseExpr(t));
             t.clear();
           }
           else if(tokens[k].type== TokenType::BEGIN_LIST_TOKEN)
           {
             int R = findEndList(k,tokens);
             if(R>(tokens.size()-2) || R==-1)
               parseError("SyntaxError","Invalid Syntax",false);
             vector<Token> sublist = {tokens.begin()+k,tokens.begin()+R+1};
             t.insert(t.end(),sublist.begin(),sublist.end());
             k = R;
           }
           else if(tokens[k].type== TokenType::L_CURLY_BRACKET_TOKEN)
           {
             int R = findRCB(k,tokens);
             if(R>(tokens.size()-2) || R==-1)
               parseError("SyntaxError","Invalid Syntax",false);
             vector<Token> subdict = {tokens.begin()+k,tokens.begin()+R+1};
             t.insert(t.end(),subdict.begin(),subdict.end());
             k = R;
           }
           else if(tokens[k].type==TokenType::ID_TOKEN && tokens[k+1].type==TokenType::LParen_TOKEN)
           {
             int R = matchRP(k,tokens);
             if(R>(tokens.size()-2) || R==-1)
               parseError("SyntaxError","Invalid Syntax",false);
             vector<Token> subfn = {tokens.begin()+k,tokens.begin()+R+1};
             t.insert(t.end(),subfn.begin(),subfn.end());
             k = R;
           }
           else
             t.push_back(tokens[k]);
         }
         if(t.size()==0)
           parseError("SyntaxError","Error expected an element after ',' ",false);
         ast->childs.push_back(parseExpr(t));
         t.clear();
         return ast;
       }
    }
    /////////////////
    k = tokens.size()-1;
    while(k>=0)
    {
      if(tokens[k].type==TokenType::RParen_TOKEN)
      {
        int i = matchRPRight(k,tokens);
        if(i==-1)
          parseError("SyntaxError","Invalid Syntax",false);
        k = i;
      }
      else if(tokens[k].type== TokenType::END_LIST_TOKEN)
      {
         int i = findBeginList(k,tokens);
         if(i==-1)
           parseError("SyntaxError","Invalid Syntax",false);
         k=i;
      }
      else if(tokens[k].type== TokenType::R_CURLY_BRACKET_TOKEN)
      {
        int i = findLCBRight(k,tokens);
        if(i==-1)
          parseError("SyntaxError","Invalid Syntax",false);
        k=i;
      }
      else if(tokens[k].type== TokenType::OP_TOKEN && (tokens[k].content=="."))
      {
        vector<Token> lhs = {tokens.begin(),tokens.begin()+k};
        vector<Token> rhs = {tokens.begin()+k+1,tokens.end()};
        if(lhs.size()==0 || rhs.size()==0)
          parseError("SyntaxError","Invalid Syntax",false);
        Node* ast = NewNode(tokens[k].content);
        ast->childs.push_back(parseExpr(lhs));
        ast->childs.push_back(parseExpr(rhs));
        return ast;
      }
      k-=1;
    }
    ////////////////
    ////////////////////
    if(tokens.size()>=2)
    {
    if(tokens[0].type== TokenType::L_CURLY_BRACKET_TOKEN && tokens.back().type==TokenType::R_CURLY_BRACKET_TOKEN)
    {
      int i = tokens.size()-1;
      if(i==tokens.size()-1)
      {
         ast = NewNode("dict");
         if(tokens.size()==2)
           return ast;
         vector<Token> Key;
         vector<Token> Value;
         string tofill = "key";
         for(int k = 1;k<=tokens.size()-2;k+=1)
         {
           if(tokens[k].type== TokenType::COMMA_TOKEN)
           {
             if(Value.size()==0)
               parseError("SyntaxError","Invalid Syntax",false);
             ast->childs.push_back(parseExpr(Value));
             Value.clear();
             tofill = "key";
           }
           else if(tokens[k].type== TokenType::COLON_TOKEN)
           {
             if(Key.size()==0)
               parseError("SyntaxError","Invalid Syntax",false);
              ast->childs.push_back(parseExpr(Key));
              Key.clear();
              tofill = "value";
           }
           else if(tokens[k].type== TokenType::BEGIN_LIST_TOKEN)
           {
             int R = findEndList(k,tokens);
             vector<Token> sublist ={tokens.begin()+k,tokens.begin()+R+1};
             if(tofill=="key")
             {
               Key.insert(Key.end(),sublist.begin(),sublist.end());
               k = R;
             }
             else
             {
               Value.insert(Value.end(),sublist.begin(),sublist.end());
               k = R;
             }
           }
           else if(tokens[k].type== TokenType::L_CURLY_BRACKET_TOKEN)
           {
             int R = findRCB(k,tokens);
             if(R==tokens.size()-1)
               parseError("SyntaxError","Invalid Syntax",false);

             vector<Token> subdict ={tokens.begin()+k,tokens.begin()+R+1};
             if(tofill=="key")
             {
               Key.insert(Key.end(),subdict.begin(),subdict.end());
               k = R;
             }
             else
             {
               Value.insert(Value.end(),subdict.begin(),subdict.end());
               k = R;
             }
          //   printf("Value.size  =%ld\n",Value.size());
           }
           else if(tokens[k].type==TokenType::ID_TOKEN && tokens[k+1].type==TokenType::LParen_TOKEN)
           {
             int R = matchRP(k,tokens);
             if(R>(tokens.size()-2) || R==-1)
               parseError("SyntaxError","Invalid Syntax",false);
             vector<Token> subfn = {tokens.begin()+k,tokens.begin()+R+1};
             if(tofill=="key")
             {
               Key.insert(Key.end(),subfn.begin(),subfn.end());
               k = R;
             }
             else
             {
               Value.insert(Value.end(),subfn.begin(),subfn.end());
               k = R;
             }
           }
           else
           {
             if(tofill=="key")
               Key.push_back(tokens[k]);
             else
               Value.push_back(tokens[k]);
           }
         }
         if(Value.size()==0)
           parseError("SyntaxError","Invalid Syntax",false);
         ast->childs.push_back(parseExpr(Value));
         Value.clear();

         return ast;
      }
      }
    }
    ////////////////////
    ////////////////////
    if(tokens[0].type== TokenType::ID_TOKEN && tokens[1].type== TokenType::LParen_TOKEN)
    {
      int i = matchRP(1,tokens);
      if(i==-1)
        parseError("SyntaxError","Invalid Syntax",false);
      if(i==tokens.size()-1)
      {
              Node* ast = NewNode("call");
      Node* args = NewNode("args");

      Node* n = NewNode("line "+to_string(tokens[0].ln));
      ast->childs.push_back(n);
      ast->childs.push_back(NewNode(tokens[0].content));

        addToFnReferenced(tokens[0].content);
        if(fntable.find("@"+tokens[0].content)!=fntable.end())
          addToFnReferenced("@"+tokens[0].content);
        //the user might be referring to function in global namespace or in current namespace
        //so we add both names to fnReferenced
        if(prefixes.size()>1)
          addToFnReferenced(prefixes.back()+tokens[0].content);

      vector<Token> T;
      vector<Token> Args = {tokens.begin()+2,tokens.end()-1};
      if(Args.size()==0)
      {
        ast->childs.push_back(args);
        return ast;
      }
      for(int k = 0;k<Args.size();k+=1)
      {
        if(Args[k].type== TokenType::COMMA_TOKEN)
        {
          if(T.size()==0)
            parseError("SyntaxError","Invalid Syntax",false);
          args->childs.push_back(parseExpr(T));
          T.clear();
        }
        else if(Args[k].type== TokenType::BEGIN_LIST_TOKEN)
        {
          int i = findEndList(k,Args);
          if(i==-1)
            parseError("SyntaxError","Invalid Syntax",false);
          vector<Token> LIST =  {Args.begin()+k,Args.begin()+i+1};
          T.insert(T.end(),LIST.begin(),LIST.end());
          k = i;
        }
        else if(Args[k].type== TokenType::LParen_TOKEN)
        {
          int i = matchRP(k,Args);
          if(i==-1)
            parseError("SyntaxError","Invalid Syntax",false);
          vector<Token> P =  {Args.begin()+k,Args.begin()+i+1};
          T.insert(T.end(),P.begin(),P.end());
          k = i;
        }
        else if(Args[k].type== TokenType::L_CURLY_BRACKET_TOKEN)
        {
          int i = findRCB(k,Args);
          if(i==-1)
            parseError("SyntaxError","Invalid Syntax",false);
          vector<Token> P =  {Args.begin()+k,Args.begin()+i+1};
          T.insert(T.end(),P.begin(),P.end());
          k = i;
        }
        else
          T.push_back(Args[k]);
      }
      if(T.size()!=0)
        args->childs.push_back(parseExpr(T));
      T.clear();
      ast->childs.push_back(args);

      return ast;
      }
    }
    ///////////////////
    //////////////
    if(tokens[0].type== TokenType::LParen_TOKEN)
    {
      int i = matchRP(0,tokens);
      if(i==-1)
        parseError("SyntaxError","Invalid Syntax",false);
      if(i==tokens.size()-1)
      {
         vector<Token> e = {tokens.begin()+1,tokens.end()-1};
         return parseExpr(e);
      }
    }
    string expr;
    for(auto e: tokens)
    {
      if(e.type==TokenType::STRING_TOKEN)
        expr+= "\"" + e.content + "\"";
      else
        expr+=e.content;
    }
    parseError("SyntaxError","Invalid syntax: "+expr,false);
    return ast;
  }

  Node* parseStmt(vector<Token> tokens,bool infunc = false,bool inclass=false)
  {
    if(tokens.size()==0)
      parseError("SyntaxError","Invalid Syntax",false);
    //length of tokens is never zero
    if(tokens.size()==1 && (tokens[0].content=="endfunc" || tokens[0].content=="endnm" || tokens[0].content=="endtry" || tokens[0].content=="endcatch" || tokens[0].content=="endfor" || tokens[0].content=="endelse" || tokens[0].content=="endwhile" || tokens[0].content =="endclass" || tokens[0].content=="endelif"  || tokens[0].content=="endif") && tokens[0].type==KEYWORD_TOKEN)
    {
      return NewNode(tokens[0].content);
    }
    bool isPrivate = false;
    if(tokens[0].type==TokenType::KEYWORD_TOKEN && tokens[0].content=="private")
    {
      isPrivate = true;
      tokens.erase(tokens.begin());
      if(!inclass)
          parseError("SyntaxError","Error use of keyword private outside class!\n",false);
      if(tokens[0].type==TokenType::KEYWORD_TOKEN && tokens[0].content=="var")
      {

      }
      else if(tokens[0].type==TokenType::KEYWORD_TOKEN && tokens[0].content=="function")
      {

      }
      else
        parseError("SyntaxError","Invalid use of keyword private\n",false);
    }
    else if(tokens[0].type==TokenType::KEYWORD_TOKEN && tokens[0].content=="public")
    {
      isPrivate = false;
      tokens.erase(tokens.begin());
      if(!inclass)
          parseError("SyntaxError","Error use of keyword public outside class!\n",false);
      if(tokens[0].type==TokenType::KEYWORD_TOKEN && tokens[0].content=="var")
      {

      }
      else if(tokens[0].type==TokenType::KEYWORD_TOKEN && tokens[0].content=="function")
      {

      }
      else
        parseError("SyntaxError","Invalid use of keyword public\n",false);
    }
    if(tokens[0].type== TokenType::KEYWORD_TOKEN  && tokens[0].content=="var")
    {
      //declare statement
      if(tokens.size()<4) // there must be minimum four tokens var x  (simplest case)
      {
          parseError("SyntaxError","invalid declare statement!\n",false);
      }
      if(tokens[1].type!= TokenType::ID_TOKEN || (tokens[2].type!= TokenType::OP_TOKEN &&  tokens[2].content!="="))
      {
        parseError("SyntaxError","invalid declare statement!\n",false);
      }
      Node* ast = NewNode("declare");
      Node* n = NewNode("line "+to_string(tokens[0].ln));
      ast->childs.push_back(n);
      if(tokens[1].content.find("::")!=string::npos)
        parseError("SyntaxError","Invalid Syntax",false);
      string fnprefix;
      for(auto e: prefixes)
        fnprefix+=e;
      if(!infunc && !inclass)
        tokens[1].content = fnprefix+tokens[1].content;
      if(isPrivate)
        tokens[1].content = "@"+tokens[1].content;
      ast->childs.push_back(NewNode("id: "+tokens[1].content));
      vector<Token> expr = {tokens.begin()+3,tokens.end()};
      if(expr[0].type==KEYWORD_TOKEN && expr[0].content == "yield")
      {
         foundYield = true;
         expr = {expr.begin()+1,expr.end()};
         ast->childs.push_back(NewNode("yield"));
         if(expr.size()==0)
            parseError("SyntaxError","Error expected expression after return keyword",false);
          Node* n = NewNode("line "+to_string(tokens[0].ln));
          ast->childs.back()->childs.push_back(n);
          ast->childs.back()->childs.push_back(parseExpr(expr));
        return ast;
      }
      ast->childs.push_back(parseExpr(expr));
      return ast;
    }
    if(tokens[0].type== TokenType::EOP_TOKEN && tokens.size()==1)
    {
      Node* ast = NewNode("EOP");
      return ast;
    }
    if(tokens[0].type== TokenType::KEYWORD_TOKEN && (tokens[0].content=="break" || tokens[0].content=="continue") && tokens.size()==1)
    {
      Node* ast = NewNode(tokens[0].content);
      ast->childs.push_back(NewNode("line "+to_string(tokens[0].ln)));
      return ast;
    }
    if(tokens[0].type== TokenType::ID_TOKEN)
    {
      if(tokens.size()>=3)
      {
        if(tokens[1].type== TokenType::LParen_TOKEN && tokens[tokens.size()-1].type== TokenType::RParen_TOKEN)
        {
          Node* ast = NewNode("call");
          Node* n = NewNode();
          n->val = "line "+to_string(tokens[0].ln);
          ast->childs.push_back(n);
          ast->childs.push_back(NewNode(tokens[0].content));
          //if()
          addToFnReferenced(tokens[0].content);
          if(fntable.find("@"+tokens[0].content)!=fntable.end())
            addToFnReferenced("@"+tokens[0].content);
          //the user might be referring to function in global namespace or in current namespace
          //so we add both names to fnReferenced
          if(prefixes.size()>1)
            addToFnReferenced(prefixes.back()+tokens[0].content);
          Node* args = NewNode("args");
          if(tokens.size()==3)
          {
            ast->childs.push_back(args);
            return ast;
          }
          vector<Token> T;
          vector<Token> Args = {tokens.begin()+2,tokens.end()-1};
          for(int k = 0;k<Args.size();k+=1)
          {
            if(Args[k].type== TokenType::COMMA_TOKEN)
            {
              if(T.size()==0)
                parseError("SyntaxError","Invalid Syntax",false);
              args->childs.push_back(parseExpr(T));
              T.clear();
            }
            else if(Args[k].type== TokenType::BEGIN_LIST_TOKEN)
            {
              int i = findEndList(k,Args);
              if(i==-1)
                parseError("SyntaxError","Invalid Syntax",false);
              vector<Token> P = {Args.begin()+k,Args.begin()+i+1};
              T.insert(T.end(),P.begin(),P.end());
              k = i;
            }
            else if(Args[k].type== TokenType::L_CURLY_BRACKET_TOKEN)
            {
              int i = findRCB(k,Args);
              if(i==-1)
                parseError("SyntaxError","Invalid Syntax",false);
              vector<Token> P = {Args.begin()+k,Args.begin()+i+1};
              T.insert(T.end(),P.begin(),P.end());
              k = i;
            }
            else if(Args[k].type== TokenType::LParen_TOKEN)
            {
              int i = matchRP(k,Args);
              if(i==-1)
                parseError("SyntaxError","Invalid Syntax",false);
              vector<Token> P = {Args.begin()+k,Args.begin()+i+1};
              T.insert(T.end(),P.begin(),P.end());
              k = i;
            }
            else
              T.push_back(Args[k]);
          }
          if(T.size()==0)
            parseError("SyntaxError","Invalid Syntax",false);
          args->childs.push_back(parseExpr(T));
          T.clear();
          ast->childs.push_back(args);
          return ast;
        }
      }
    }
    if(tokens[0].type==TokenType::KEYWORD_TOKEN && tokens[0].content=="class")
    {
      if(tokens.size()!=2 && tokens.size()!=4)
        parseError("SyntaxError","Invalid Syntax",false);
      if(tokens[1].type!=ID_TOKEN)
        parseError("SyntaxError","Invalid Syntax",false);
      bool extendedClass=false;
      if(tokens.size()==4)
      {
        if(tokens[2].type!=TokenType::KEYWORD_TOKEN || tokens[3].type!=TokenType::ID_TOKEN)
          parseError("SyntaxError","Invalid Syntax",false);
        if(tokens[2].content!="extends")
            parseError("SyntaxError","Invalid Syntax",false);
        extendedClass = true;
      }
      Node* ast;
      if(!extendedClass)
        ast = NewNode("class");
      else
        ast = NewNode("extclass");
      Node* n = NewNode("line "+to_string(line_num));
      ast->childs.push_back(n);
      if(tokens[1].content.find("::")!=string::npos)
        parseError("SyntaxError","Invalid Syntax",false);
      string fnprefix;
      for(auto e: prefixes)
        fnprefix+=e;
      if(!infunc)
        tokens[1].content = fnprefix+tokens[1].content;
      ast->childs.push_back(NewNode(tokens[1].content));
      if(extendedClass)
        ast->childs.push_back(parseExpr(vector<Token>{tokens.begin()+3,tokens.end()}));
      return ast;
    }
    if(tokens[0].type== TokenType::KEYWORD_TOKEN && (tokens[0].content=="while" || tokens[0].content=="dowhile"))
    {
      if(tokens.size()>=4)
      {
        if(tokens[1].type== TokenType::LParen_TOKEN  && tokens.back().type== TokenType::RParen_TOKEN)
        {
          vector<Token> expr = {tokens.begin()+2,tokens.end()-1};
          Node* ast = NewNode(tokens[0].content);
          Node* n = NewNode("line "+to_string(tokens[0].ln));
          ast->childs.push_back(n);
          ast->childs.push_back(parseExpr(expr));
          return ast;
        }
      }
    }
    if(tokens[0].type==TokenType::KEYWORD_TOKEN && tokens[0].content=="for")
    {
        if(tokens.size()<3)
          parseError("SyntaxError","Invalid Syntax",false);
        if(tokens[1].type!=TokenType::LParen_TOKEN || tokens.back().type!=TokenType::RParen_TOKEN)
        {
          parseError("SyntaxError","Invalid Syntax",false);
        }
        tokens = {tokens.begin()+2,tokens.end()-1};

        Token t;
        t.type = TokenType::KEYWORD_TOKEN;
        t.content = "to";
        int i = findToken(t,0,tokens);
        if(i==-1)
        {
          t.type = TokenType::Ellipsis_TOKEN;
          t.content = "..";
          i = findToken(t,0,tokens);
          if(i==-1)
            parseError("SyntaxError","Invalid Syntax",false);
        }
        vector<Token> init = {tokens.begin(),tokens.begin()+i};

        int p = i;
        t.content = "step";
        t.type = TokenType::KEYWORD_TOKEN;
        i = findToken(t,i+1,tokens);
        if(i==-1)
        parseError("SyntaxError","Invalid Syntax",false);
        vector<Token> endpoint = {tokens.begin()+p+1,tokens.begin()+i};
        vector<Token> inc = {tokens.begin()+i+1,tokens.end()};
        bool decl = false;
        if(init[0].type==TokenType::KEYWORD_TOKEN && init[0].content=="var" )
        {
          init.erase(init.begin());
          decl = true;
        }
        if(init[0].type==TokenType::ID_TOKEN && init[1].type==TokenType::OP_TOKEN && init[1].content=="=")
        {
          vector<Token> initExpr = {init.begin()+2,init.end()};
          Node* ast = NewNode("for");
          Node* n = NewNode("line "+to_string(tokens[0].ln));
          ast->childs.push_back(n);
          if(decl)
            ast->childs.push_back(NewNode("decl"));
          else
            ast->childs.push_back(NewNode("nodecl"));
          ast->childs.push_back(NewNode(init[0].content));
          ast->childs.push_back(parseExpr(initExpr));
          ast->childs.push_back(parseExpr(endpoint));
          ast->childs.push_back(parseExpr(inc));

          return ast;
        }
    }
    if(tokens[0].type==TokenType::KEYWORD_TOKEN && tokens[0].content=="foreach")
    {
      if(tokens.size()<3)
        parseError("SyntaxError","Invalid Syntax",false);

      if(tokens[1].type!=TokenType::LParen_TOKEN || tokens.back().type!=TokenType::RParen_TOKEN)
        parseError("SyntaxError","Invalid Syntax",false);
      tokens = {tokens.begin()+2,tokens.end()-1};
      if(tokens.size()<4)
        parseError("SyntaxError","Invalid Syntax",false);
      if(tokens[0].type!=TokenType::KEYWORD_TOKEN || tokens[0].content!="var" || tokens[1].type!=TokenType::ID_TOKEN || tokens[2].type!=TokenType::COLON_TOKEN)
        parseError("SyntaxError","Invalid Syntax",false);
      vector<Token> expr = {tokens.begin()+3,tokens.end()};

      Node* exprAST = parseExpr(expr);
      Node* ast = NewNode("foreach");
      Node* n = NewNode("line "+to_string(tokens[0].ln));
      ast->childs.push_back(n);
      ast->childs.push_back(NewNode(tokens[1].content));
      ast->childs.push_back(exprAST);
      Token Q;
      Q.type = TokenType::NUM_TOKEN;
      Q.content = "-1";
      vector<Token> E = {Q};
      ast->childs.push_back(parseExpr(E));
      return ast;
    }
    if(tokens[0].type==TokenType::KEYWORD_TOKEN && tokens[0].content=="namespace")
    {
      if(tokens.size()!=2)
        parseError("SyntaxError","Invalid Syntax",false);
      if(tokens[1].type!=ID_TOKEN)
        parseError("SyntaxError","Invalid Syntax",false);
      Node* ast = NewNode("namespace");
      Node* n = NewNode("line "+to_string(tokens[0].ln));
      ast->childs.push_back(n);
      string fnprefix;
      if(tokens[1].content.find("::")!=string::npos)
        parseError("SyntaxError","Invalid Syntax",false);

      ast->childs.push_back(NewNode(tokens[1].content));
      return ast;
    }
    if(tokens[0].type== TokenType::KEYWORD_TOKEN && tokens[0].content=="if")
    {
      if(tokens.size()>=4)
      {
        if(tokens[1].type== TokenType::LParen_TOKEN  && tokens.back().type== TokenType::RParen_TOKEN)
        {
          vector<Token> expr = {tokens.begin()+2,tokens.end()-1};
          Node* ast = NewNode("if");
          Node* n = NewNode("line "+to_string(tokens[0].ln));
          Node* f = NewNode("conditions");
          f->childs.push_back(parseExpr(expr));
          ast->childs.push_back(n);
          ast->childs.push_back(f);
          return ast;
        }
      }
    }
    if(tokens[0].type== TokenType::KEYWORD_TOKEN && tokens[0].content=="import")
      {
          Node* ast = NewNode("import");
          vector<Token> t;
          if(tokens.size()==2)
          {
          if(tokens[1].type==ID_TOKEN)
          {
            t.push_back(tokens[1]);
            Node* n = new Node;
            n->val = "line ";
            n->val+=to_string(tokens[0].ln);
            ast->childs.push_back(n);
            ast->childs.push_back(parseExpr(t));
            return ast;
          }
          if(tokens[1].type== TokenType::STRING_TOKEN)
          {
            t.push_back(tokens[1]);
            Node* n = new Node;
            n->val = "line ";
            n->val+=to_string(tokens[0].ln);
            ast->childs.push_back(n);
            ast->childs.push_back(NewNode("string: "+tokens[1].content));
            return ast;
          }
          else
          {
                  //line_num = tokens[1].ln;
                  parseError("SyntaxError","Invalid Syntax",false);
          }
          }
          else if(tokens.size()==6)
          {
             if(tokens[1].type!=ID_TOKEN || tokens[1].content!="std" || tokens[2].type!=OP_TOKEN || tokens[2].content!="/" || tokens[3].type!=ID_TOKEN || tokens[4].type!=OP_TOKEN || tokens[4].content!="." || tokens[5].type!=ID_TOKEN || tokens[5].content!="plt")
               parseError("SyntaxError","Invalid Syntax",false);
            #ifdef BUILD_FOR_WINDOWS
            tokens[3].content="C:\\plutonium\\std\\"+tokens[3].content+".plt";
            #else
            tokens[3].content="/opt/plutonium/std/"+tokens[3].content+".plt";
            #endif
            Node* n = new Node;
            n->val = "line ";
            n->val+=to_string(tokens[0].ln);
            ast->childs.push_back(n);
            ast->childs.push_back(NewNode("string: "+tokens[3].content));
            return ast;
          }
          else
          {
              parseError("SyntaxError","Invalid Syntax",false);
          }
      }
      if(tokens[0].type== TokenType::KEYWORD_TOKEN && tokens[0].content=="return")
      {
          Node* ast = NewNode("return");
          vector<Token> t = {tokens.begin()+1,tokens.end()};
          if(t.size()==0)
            parseError("SyntaxError","Error expected expression after return keyword",false);
          Node* n = NewNode("line "+to_string(tokens[0].ln));
          ast->childs.push_back(n);
          ast->childs.push_back(parseExpr(t));
          return ast;
      }
      if(tokens[0].type== TokenType::KEYWORD_TOKEN && tokens[0].content=="yield")
      {
          foundYield = true;
          Node* ast = NewNode("yield");
          vector<Token> t = {tokens.begin()+1,tokens.end()};
          if(t.size()==0)
            parseError("SyntaxError","Error expected expression after yield keyword",false);
          Node* n = NewNode("line "+to_string(tokens[0].ln));
          ast->childs.push_back(n);
          ast->childs.push_back(parseExpr(t));
          return ast;
      }
    if(tokens[0].type== TokenType::KEYWORD_TOKEN && tokens[0].content=="function")
    {
      if(tokens.size()<4)
        parseError("SyntaxError","Invalid Syntax",false);
      if(tokens.back().type==L_CURLY_BRACKET_TOKEN)
        tokens.pop_back();
      if(tokens[1].type!=ID_TOKEN || tokens.back().type!=TokenType::RParen_TOKEN)
        parseError("SyntaxError","Invalid Syntax",false);
        if(tokens[1].content.find("::")!=string::npos)
          parseError("SyntaxError","Invalid Syntax",false);
        string fnprefix;
        for(auto e: prefixes)
          fnprefix+=e;
      if(!inclass)
      tokens[1].content = fnprefix+tokens[1].content;
      if(isPrivate)
        tokens[1].content = "@" + tokens[1].content;
      Node* ast = NewNode("func "+tokens[1].content);
      Node* n = NewNode("line "+to_string(tokens[0].ln));
      ast->childs.push_back(n);
      ast->childs.push_back(NewNode("args"));

      if(tokens.size()==4)
        return ast;
      vector<Token> args = {tokens.begin()+3,tokens.end()-1};
      if(args.size()<2)
        parseError("SyntaxError","Invalid Syntax",false);
      size_t k = 0;
      bool found_default = false;//found any default argument
      while(k<args.size())
      {
        if(args[k].type== TokenType::KEYWORD_TOKEN && args[k].content=="var")
        {
          if(k==args.size()-1)
            parseError("SyntaxError","Invalid Syntax",false);
          if(args[k+1].type!= TokenType::ID_TOKEN)
            parseError("SyntaxError","Invalid Syntax",false);
          ast->childs[1]->childs.push_back(NewNode(args[k+1].content));
          k+=1;
          if(k<args.size()-1)// or k+1 < args.size()
          {
            k+=1;
            if(args[k].type!= TokenType::COMMA_TOKEN)
              parseError("SyntaxError","Invalid Syntax",false);
            if(k==args.size()-1)
              parseError("SyntaxError","Invalid Syntax",false);
            k+=1;
            continue;
          }
        }
        else
          parseError("SyntaxError","Invalid Syntax",false);
        k+=1;
      }
      return ast;
    }


     int k=0;
      while(k<tokens.size())
      {
          if(tokens[k].type!= TokenType::OP_TOKEN)
          {
              k+=1;
              continue;
          }
          if(tokens[k].content=="=")
          {
            Node* ast = NewNode(tokens[k].content);
            vector<Token> left = {tokens.begin(),tokens.begin()+k};
            if(left.size()==0)
              parseError("SyntaxError","Invalid Syntax",false);
            Node* n = NewNode("line "+to_string(tokens[k].ln));
            ast->childs.push_back(n);
            ast->childs.push_back(parseExpr(left));
            vector<Token> right = {tokens.begin()+k+1,tokens.end()};
            if(right.size()==0)
              parseError("SyntaxError","Invalid Syntax",false);
            if(right[0].type==KEYWORD_TOKEN && right[0].content=="yield")
            {
              right = {right.begin()+1,right.end()};
              ast->childs.push_back(NewNode("yield"));
              if(right.size()==0)
                parseError("SyntaxError","Error expected expression after return keyword",false);
              Node* n = NewNode("line "+to_string(tokens[k+1].ln));
              ast->childs.back()->childs.push_back(n);
              ast->childs.back()->childs.push_back(parseExpr(right));
              foundYield = true;
              return ast;
            }
            ast->childs.push_back(parseExpr(right));
            return ast;
          }
          else if(tokens[k].content=="+=" || tokens[k].content=="-=" || tokens[k].content=="/=" || tokens[k].content=="*=" || tokens[k].content=="^=" || tokens[k].content=="%=" || tokens[k].content=="|=" || tokens[k].content=="&=" || tokens[k].content=="<<=" || tokens[k].content==">>=")
          {
             Node* ast = NewNode("=");
             vector<Token> left = {tokens.begin(),tokens.begin()+k};
             //line_num  = tokens[k].ln;
             if(left.size()==0 )
               parseError("SyntaxError","Invalid Syntax",false);
             Node* n = new Node;
             n->val = "line ";
             n->val+=to_string(tokens[k].ln);
             ast->childs.push_back(n);
             ast->childs.push_back(parseExpr(left));
             Token o;
             o.type = OP_TOKEN;
             o.content+=tokens[k].content[0];
             left.push_back(o);
             vector<Token> right = {tokens.begin()+k+1,tokens.end()};
             if(right.size()==0)
               parseError("SyntaxError","Invalid Syntax",false);
             
             right.insert(right.begin(),left.begin(),left.end());
             ast->childs.push_back(parseExpr(right));
             return ast;
          }
          k+=1;
      }
      if(tokens[0].type== TokenType::ID_TOKEN && tokens.size()>=5)
      {
         if(tokens[1].type== TokenType::OP_TOKEN && tokens[1].content==".")
         {
           Node* line = NewNode("line "+to_string(tokens[0].ln));
           Node* ast = parseExpr(tokens);
           ast->childs.insert(ast->childs.begin(),line);
           return ast;
         }
      }
      if(tokens[0].type==TokenType::KEYWORD_TOKEN && tokens[0].content=="try")
      {
           Node* line = NewNode("line "+to_string(tokens[0].ln));
           Node* ast = NewNode("trycatch");
           ast->childs.push_back(line);
           return ast;
      }
      if(tokens[0].type==TokenType::KEYWORD_TOKEN && tokens[0].content=="throw")
      {
        if(tokens.size()!=4)
        {
          parseError("SyntaxError","Invalid Syntax",false);
        }
        if(tokens[1].type!=TokenType::NUM_TOKEN || tokens[2].type!=TokenType::COMMA_TOKEN || tokens[3].type!=TokenType::STRING_TOKEN)
          parseError("SyntaxError","Invalid Syntax",false);
        vector<Token> expr = {tokens.begin()+1,tokens.end()};
        Node* line = NewNode("line "+to_string(tokens[0].ln));
        Node* ast = NewNode("throw");
        ast->childs.push_back(line);
        ast->childs.push_back(parseExpr(vector<Token>{tokens[1]}));
        ast->childs.push_back(parseExpr(vector<Token>{tokens[3]}));
        return ast;
      }

    parseError("SyntaxError","Unknown statement",false);
    return nullptr;//to avoid compiler warning otherwise the parseError function exits after printing error message

  }

  Node* parse(const vector<Token>& tokens,bool inwhile=false,bool infunc=false,bool inclass=false)
  {
      int k = 0;
      Node* ast = nullptr;
      Token ElseTok,elif,newlinetok,bgscope,endscope;
      int start = 0;
      Node* e = nullptr;//e points to the lowest rightmost node of ast
      Node* Final = nullptr;
      while(k<tokens.size())
      {
          if(tokens[k].type== TokenType::NEWLINE_TOKEN)
          {

              vector<Token> line = {tokens.begin()+start,tokens.begin()+k};
              if(line.size()==0)
              {
                start+=1;
                k+=1;
                continue;
              }
              line_num = line[0].ln;
              if(line[line.size()-1].type== TokenType::L_CURLY_BRACKET_TOKEN)
                  line.pop_back();
              if(line.size()==0)
              {
                    start+=1;
                    k+=1;
                    continue;
              }

              ast = parseStmt(line,infunc,inclass);
              if(inclass && ast->val!="declare" && substr(0,4,ast->val)!="func " && ast->val!="endclass" && ast->val!="endnm" && ast->val!="endfunc")
              {
                parseError("SyntaxError","Error classes are allowed to have member variables and functions only!",false);
              }
              if(ast->val=="if")
              {
                  ElseTok.type = TokenType::KEYWORD_TOKEN;
                  ElseTok.content = "else";
                  elif.type = TokenType::KEYWORD_TOKEN;
                  elif.content = "else if";
                  newlinetok.type = TokenType::NEWLINE_TOKEN;
                  newlinetok.content = "\n";
                  bgscope.type = TokenType::L_CURLY_BRACKET_TOKEN;
                  bgscope.content = "{";
                  endscope.type = TokenType::R_CURLY_BRACKET_TOKEN;
                  endscope.content = "}";
                  Token lptok;
                  lptok.type = TokenType::LParen_TOKEN;
                  lptok.content = "(";
                 int j = findTokenConsecutive(bgscope,start+line.size(),tokens);
                 int i;
                 if(j!=-1)
                   i = findRCB(j,tokens);
                 if(j==-1)
                 {
                     j = findTokenConsecutive(newlinetok,start+line.size(),tokens);
                     i = findToken(newlinetok,j+1,tokens);
                     line_num = tokens[0].ln;
                     if(i==-1)
                       parseError("SyntaxError","Error expected code block or line after if!",false);


                 }
                 vector<Token> block = {tokens.begin()+j+1,tokens.begin()+i};//if block
                 if(block.size()==1 && block[0].type==KEYWORD_TOKEN && (block[0].content=="endfor" || tokens[0].content=="endtry" || tokens[0].content=="endcatch" || block[0].content=="endwhile" || block[0].content=="endnm" || block[0].content=="endclass" || block[0].content=="endfunc" || block[0].content=="endelse" || block[0].content=="endelif" ||  block[0].content=="endif"))
                 {
                       parseError("SyntaxError","Error use brackets {} after if!",false);
                 }
               //  printf("ifblock size = %ld\nisNEWLINE_TOKEN = %d\n%s\n",block.size(),block[0].type==KEYWORD_TOKEN,block[0].content.c_str());
                 removeUselessNewlines(block);
                 vector<vector<Token>> elifBlocks;
                 vector<vector<Token>> elifConditions;
                 j = findTokenConsecutive(elif,i+1,tokens);

                 bool foundelif = false;
                 bool foundElse = false;
                 if(j!=-1)
                 {
                     foundelif = true;
                 }
                 while(j!=-1)
                 {

                   int p = findTokenConsecutive(lptok,j+1,tokens);
                   if(p==-1)
                   {

                      line_num = tokens[j].ln;
                      parseError("SyntaxError","Error expected '(' after else if",false);
                   }
                   int l = matchRP(p,tokens);

                   vector<Token> condition = {tokens.begin()+p+1,tokens.begin()+l};
                   elifConditions.push_back(condition);
                   j = findTokenConsecutive(bgscope,l+1,tokens);
                   if(j==-1)
                   {
                     j = findTokenConsecutive(newlinetok,l+1,tokens);
                     i = findToken(newlinetok,j+1,tokens);
                       line_num = tokens[p].ln;
                       j-=1;
                       if(i==-1 )
                       parseError("SyntaxError","Error expected  code block or line after else if",false);
                   }
                   else
                     i = findRCB(j,tokens);
                   vector<Token> elifBlock = {tokens.begin()+j+1+1,tokens.begin()+i};
                   if(elifBlock.size()==1 && elifBlock[0].type==KEYWORD_TOKEN && (elifBlock[0].content=="endfor" || elifBlock[0].content=="endwhile" || elifBlock[0].content=="endnm" || elifBlock[0].content=="endclass" || elifBlock[0].content=="endfunc" || elifBlock[0].content=="endelse" || elifBlock[0].content=="endelif" ||  elifBlock[0].content=="endif"))
                 {
                       parseError("SyntaxError","Error use brackets {} after else if!",false);
                 }
                   elifBlocks.push_back(elifBlock);
                   j = findTokenConsecutive(elif,i+1,tokens);
                 }
                 j = findTokenConsecutive(ElseTok,i+1,tokens);
                 if(j!=-1)
                 {
                     foundElse= true;
                         line_num = tokens[j].ln;
                     int e = j;
                     j = findTokenConsecutive(bgscope,j+1,tokens);
                     if(j==-1)
                     {
                         j = findTokenConsecutive(newlinetok,e+1,tokens);
                         i = findToken(newlinetok,j+1,tokens);
                         if(i==-1)
                           parseError("SyntaxError","Error expected code block or line after else",false);
                     }
                     else
                       i = findRCB(j,tokens);
                 }
                 if(foundelif && foundElse)
                 {
                   ast->val = "ifelifelse";
                   vector<Token> elseBlock = {tokens.begin()+j+1,tokens.begin()+i};
                   if(elseBlock.size()==1 && elseBlock[0].type==KEYWORD_TOKEN && (elseBlock[0].content=="endfor" || elseBlock[0].content=="endwhile" || elseBlock[0].content=="endnm" || elseBlock[0].content=="endclass" || elseBlock[0].content=="endfunc" || elseBlock[0].content=="endelse" || elseBlock[0].content=="endelif" ||  elseBlock[0].content=="endif"))
                 {
                       parseError("SyntaxError","Error use brackets {} after else!",false);
                 }
                   removeUselessNewlines(elseBlock);
                   for(int a=0;a<elifConditions.size();a++)
                   {
                       ast->childs[1]->childs.push_back(parseExpr(elifConditions[a]));
                   }
                   Node* r;
                   Token t;
                   t.content = "endif";
                   t.type=KEYWORD_TOKEN;
                   if(block.size()!=0)
                     block.push_back(newlinetok);
                   block.push_back(t);
                   block.push_back(newlinetok);
                   if(elseBlock.size()!=0)
                     elseBlock.push_back(newlinetok);
                   t.content = "endelse";
                   elseBlock.push_back(t);
                   elseBlock.push_back(newlinetok);
                   ast->childs.push_back(parse(block,inwhile,infunc));
                   t.content = "endelif";
                   for(int a=0;a<elifBlocks.size();a++)
                   {
                      vector<Token> elifBlock = elifBlocks[a];
                      removeUselessNewlines(elifBlock);
                      if(elifBlock.size()!=0)
                        elifBlock.push_back(newlinetok);
                      elifBlock.push_back(t);
                      elifBlock.push_back(newlinetok);
                      Node* n = parse(elifBlock,inwhile,infunc);
                      ast->childs.push_back(n);
                   }
                   ast->childs.push_back(parse(elseBlock,inwhile,infunc));
                   if(start==0)
                   {
                     Final = ast;
                     e = ast;
                   }
                   else
                   {
                     e->childs.push_back(ast);
                     e = e->childs.back();
                   }
                   start=i+1;
                   k = i;
                 }
                 else if(foundelif && !foundElse)
                 {
                   ast->val = "ifelif";
                   for(int a=0;a<elifConditions.size();a++)
                   {
                       ast->childs[1]->childs.push_back(parseExpr(elifConditions[a]));
                   }
                   Node* r;
                   Token t;
                   t.content = "endif";
                   t.type=KEYWORD_TOKEN;
                   if(block.size()!=0)
                     block.push_back(newlinetok);
                   block.push_back(t);
                   block.push_back(newlinetok);
                   ast->childs.push_back(parse(block,inwhile,infunc));
                   t.content = "endelif";
                   for(int a=0;a<elifBlocks.size();a++)
                   {
                      vector<Token> elifBlock = elifBlocks[a];
                      removeUselessNewlines(elifBlock);
                      if(elifBlock.size()!=0)
                        elifBlock.push_back(newlinetok);
                      elifBlock.push_back(t);
                      elifBlock.push_back(newlinetok);
                      Node* n = parse(elifBlock,inwhile,infunc);
                      ast->childs.push_back(n);
                   }
                   if(start==0)
                   {
                     Final = ast;
                     e = ast;
                   }
                   else
                   {
                      e->childs.push_back(ast);
                      e = e->childs.back();
                   }
                   start=i+1;
                   k = i;
                 }
                 else if(!foundelif && foundElse)
                 {
                   ast->val = "ifelse";
                   vector<Token> elseBlock = {tokens.begin()+j+1,tokens.begin()+i};
                   removeUselessNewlines(elseBlock);
                   Node* r;
                   Token t;
                   t.content = "endif";
                   t.type = KEYWORD_TOKEN;
                   if(block.size()!=0)
                     block.push_back(newlinetok);
                   block.push_back(t);
                   block.push_back(newlinetok);
                   t.content = "endelse";
                   if(elseBlock.size()!=0)
                     elseBlock.push_back(newlinetok);
                   elseBlock.push_back(t);
                   elseBlock.push_back(newlinetok);
                   ast->childs.push_back(parse(block,inwhile,infunc));
                   ast->childs.push_back(parse(elseBlock,inwhile,infunc));
                   if(start==0)
                   {
                     Final = ast;
                     e = ast;
                   }
                   else
                   {
                     e->childs.push_back(ast);
                     e = e->childs.back();
                   }
                   start=i+1;
                   k = i;
                 }
                 else if(!foundelif && !foundElse)
                 {
                     ast->val = "if";
                     Node* r;
                     Token t;
                     t.content = "endif";
                     t.type = KEYWORD_TOKEN;
                     if(block.size()!=0)
                       block.push_back(newlinetok);
                     block.push_back(t);
                     block.push_back(newlinetok);
                     ast->childs.push_back(parse(block,inwhile,infunc));
                     if(start==0)
                     {
                       Final = ast;
                       e = ast;
                     }
                     else
                     {
                       e->childs.push_back(ast);
                       e = e->childs.back();
                     }
                     start=i+1;
                     k = i;
                 }
              }
              else if(ast->val.substr(0,5)=="func ")
              {
              //   printf("parser found a function %s\n",ast->val.substr(5).c_str());
                 bgscope.content = "{";
                 bgscope.type=L_CURLY_BRACKET_TOKEN;
                 int j = findTokenConsecutive(bgscope,start+line.size(),tokens);
                 int i = -1;
                 if(j==-1)
                 {
                     newlinetok.content = "\n";
                     newlinetok.type=NEWLINE_TOKEN;
                     j = findTokenConsecutive(newlinetok,start+line.size(),tokens);
                     i = findToken(newlinetok,j+1,tokens);
                 }
                 else
                   i = findRCB(j,tokens);
                 vector<Token> block = {tokens.begin()+j+1,tokens.begin()+i};
                 if(block.size()==1 && block[0].type==KEYWORD_TOKEN && (block[0].content=="endfor" || tokens[0].content=="endtry" || tokens[0].content=="endcatch" || block[0].content=="endwhile" || block[0].content=="endnm" || block[0].content=="endclass" || block[0].content=="endfunc" || block[0].content=="endelse" || block[0].content=="endelif" || block[0].content=="endif"))
                 {
                       parseError("SyntaxError","Error expected code block or line after function definition!",false);
                 }
                 removeUselessNewlines(block);
                 newlinetok.type = NEWLINE_TOKEN;
                 newlinetok.content = "\n";
                 if(block.size()!=0)
                 block.push_back(newlinetok);
                 Token t;
                 t.type=KEYWORD_TOKEN;
                 t.content = "endfunc";
                 block.push_back(t);
                 block.push_back(newlinetok);
                 size_t before = fnReferenced->size();
                 fntable.emplace(ast->val.substr(5),vector<string>{});
                 foundYield = false;
                 ast->childs.push_back(parse(block,inwhile,true));
                 if(foundYield)
                   ast->val = "gen "+ast->val.substr(5);
              //   printf("dependencies: ");
                 size_t increase = fnReferenced->size()-before;
                 for(int i=1;i<=increase;i+=1)
                 {
                   string e = fnReferenced->back();
                   fnReferenced->pop_back();
                  // printf("%s  ",e.c_str());
                   if(e!=ast->val.substr(5))
                     fntable[ast->val.substr(5)].push_back(e);
                 }
                //printf("\n");
                 if(start==0)
                 {
                   Final = ast;
                   e = ast;
                 }
                 else
                 {
                  e->childs.push_back(ast);
                  e = e->childs.back();
                 }
                 start=i+1;
                 k = i;
              }
              else if(ast->val=="class" || ast->val=="extclass")
              {
              //   printf("parsing class\n");
                 bgscope.content = "{";
                 bgscope.type=L_CURLY_BRACKET_TOKEN;
                 int j = findTokenConsecutive(bgscope,start+line.size(),tokens);
                 int i = -1;
                 if(j==-1)
                 {
                    parseError("SyntaxError","Error expected brackets {} after class definition!",false);
                 }
                 else
                   i = findRCB(j,tokens);
                 vector<Token> block = {tokens.begin()+j+1,tokens.begin()+i};
                 removeUselessNewlines(block);
                 newlinetok.type = NEWLINE_TOKEN;
                 newlinetok.content = "\n";
                 if(block.size()!=0)
                 block.push_back(newlinetok);
                 Token t;
                 t.type=KEYWORD_TOKEN;
                 t.content = "endclass";
                 block.push_back(t);
                 block.push_back(newlinetok);
                 if(ast->val=="class")
                   ast->childs.push_back(parse(block,inwhile,false,true));
                 else
                   ast->childs.insert(ast->childs.begin()+2,parse(block,inwhile,false,true));
                 if(start==0)
                 {
                   Final = ast;
                   e = ast;
                 }
                 else
                 {
                  e->childs.push_back(ast);
                  e = e->childs.back();
                 }
                 start=i+1;
                 k = i;
              }
              else if(ast->val=="namespace")
              {
                 bgscope.content = "{";
                 bgscope.type=L_CURLY_BRACKET_TOKEN;
                 int j = findTokenConsecutive(bgscope,start+line.size(),tokens);
                 int i = -1;
                 if(j==-1)
                 {
                     parseError("SyntaxError","Error expected {} after namespace keyword\n",false);
                 }
                 else
                   i = findRCB(j,tokens);
                 vector<Token> block = {tokens.begin()+j+1,tokens.begin()+i};
                 removeUselessNewlines(block);
                 newlinetok.type = NEWLINE_TOKEN;
                 newlinetok.content = "\n";
                 if(block.size()!=0)
                 block.push_back(newlinetok);
                 Token t;
                 t.type=KEYWORD_TOKEN;
                 t.content = "endnm";//end namespace
                 block.push_back(t);
                 block.push_back(newlinetok);

                 prefixes.push_back(ast->childs[1]->val+"::");//prefix each identifier in namespace block
                 //with "namespaceName::"
                 ast->childs.push_back(parse(block,inwhile,false));
                 prefixes.pop_back();
                 if(start==0)
                 {
                   Final = ast;
                   e = ast;
                 }
                 else
                 {
                  e->childs.push_back(ast);
                  e = e->childs.back();
                 }
                 start=i+1;
                 k = i;
              }
              else if(ast->val=="while" || ast->val=="dowhile" || ast->val=="for" || ast->val=="foreach")//loops
              {
            //    printf("parsing loop\n");
                 bgscope.content ="{";
                 bgscope.type= L_CURLY_BRACKET_TOKEN;
                 int j = findTokenConsecutive(bgscope,start+line.size(),tokens);
                 int i;
                 if(j==-1)
                 {
                     newlinetok.content = "\n";
                     newlinetok.type=NEWLINE_TOKEN;
                     j = findTokenConsecutive(newlinetok,start+line.size(),tokens);
                     i = findToken(newlinetok,j+1,tokens);
                     if(i==-1)
                      parseError("SyntaxError","Error expected a code block or line after while!",false);
                 }
                 else
                   i = findRCB(j,tokens);
                 vector<Token> block = {tokens.begin()+j+1,tokens.begin()+i};
                 if(block.size()==1 && block[0].type==KEYWORD_TOKEN && (block[0].content=="endfor" || tokens[0].content=="endtry" || tokens[0].content=="endcatch" || block[0].content=="endwhile" || block[0].content=="endnm" || block[0].content=="endclass" || block[0].content=="endfunc" || block[0].content=="endelse" || block[0].content=="endelif" ||  block[0].content=="endif"))
                 {
                       parseError("SyntaxError","Error use brackets {} after "+ast->val,false);
                 }
               //  printf("loop block size = %ld\n",block.size());
                 removeUselessNewlines(block);
                 Token t;
                 if(ast->val=="while" || ast->val=="dowhile")
                   t.content = "endwhile";
                 else
                   t.content = "endfor";
                 t.type = KEYWORD_TOKEN;
                 Node*  p = ast;
                 Token newlinetok;
                 newlinetok.type = NEWLINE_TOKEN;
                 newlinetok.content = "\n";
                 if(block.size()!=0)
                   block.push_back(newlinetok);
                 block.push_back(t);
                 block.push_back(newlinetok);
                 ast->childs.push_back(parse(block,true,infunc));//inwhile = true
                 if(start==0)
                 {
                   Final = ast;
                   e = Final;
                 }
                 else
                 {
                   e->childs.push_back(ast);
                   e = e->childs.back();
                 }
                 start=i+1;
                 k = i;
              }
              else if(ast->val=="trycatch")//try catch
              {
                 bgscope.content ="{";
                 bgscope.type= L_CURLY_BRACKET_TOKEN;
                 int j = findTokenConsecutive(bgscope,start+line.size(),tokens);
                 int i;
                 if(j==-1)
                 {
                     newlinetok.content = "\n";
                     newlinetok.type=NEWLINE_TOKEN;
                     j = findTokenConsecutive(newlinetok,start+line.size(),tokens);
                     i = findToken(newlinetok,j+1,tokens);
                     if(i==-1)
                      parseError("SyntaxError","Error expected a code block or line after try!",false);
                 }
                 else
                   i = findRCB(j,tokens);
                 vector<Token> block = {tokens.begin()+j+1,tokens.begin()+i};
                 if(block.size()==1 && block[0].type==KEYWORD_TOKEN && (block[0].content=="endfor" || block[0].content=="endwhile" || block[0].content=="endnm" || block[0].content=="endclass" || block[0].content=="endfunc" || tokens[0].content=="endtry" || tokens[0].content=="endcatch" || block[0].content=="endelse" || block[0].content=="endelif" ||  block[0].content=="endif"))
                 {
                       parseError("SyntaxError","Error use brackets {} after "+ast->val,false);
                 }
                 removeUselessNewlines(block);
                 Token CATCH_TOK;
                 CATCH_TOK.type = TokenType::KEYWORD_TOKEN;
                 CATCH_TOK.content = "catch";
                 string catchId ;
                 j = findTokenConsecutive(CATCH_TOK,i+1,tokens);
                 if(j!=-1)
                 {
                     line_num = tokens[j].ln;
                     if(j+4>=tokens.size())
                     {
                       parseError("SyntaxError","Invalid Syntax",false);
                     }
                     j+=1;
                     if(tokens[j].type!=TokenType::LParen_TOKEN)
                       parseError("SyntaxError","Invalid Syntax",false);
                     j+=1;
                     if(tokens[j].type!=TokenType::ID_TOKEN)
                       parseError("SyntaxError","Invalid Syntax",false);
                     catchId = tokens[j].content;
                     j+=1;
                     if(tokens[j].type!=TokenType::RParen_TOKEN)
                       parseError("SyntaxError","Invalid Syntax",false);
                     j+=1;
                     j = findTokenConsecutive(bgscope,j,tokens);
                     int e = j;
                     if(j==-1)
                     {
                         j = findTokenConsecutive(newlinetok,e+1,tokens);
                         i = findToken(newlinetok,j+1,tokens);
                         if(i==-1)
                           parseError("SyntaxError","Error expected code block or line after catch",false);
                     }
                     else
                       i = findRCB(j,tokens);
                 }
                 else
                 {
                   parseError("SyntaxError","Error expected catch block after try{}!",false);
                 }
                 vector<Token> catchBlock = {tokens.begin()+j+1,tokens.begin()+i};
                 removeUselessNewlines(catchBlock);
                 Token t;
                 t.content = "endtry";
                 t.type = KEYWORD_TOKEN;
                 Node*  p = ast;
                 Token newlinetok;
                 newlinetok.type = NEWLINE_TOKEN;
                 newlinetok.content = "\n";
                 if(block.size()!=0)
                   block.push_back(newlinetok);
                 block.push_back(t);
                 block.push_back(newlinetok);
                 t.content = "endcatch";
                 if(catchBlock.size()!=0)
                   catchBlock.push_back(newlinetok);
                 catchBlock.push_back(t);
                 catchBlock.push_back(newlinetok);
                 ast->childs.push_back(NewNode(catchId));
                 ast->childs.push_back(parse(block,false,infunc));
                 ast->childs.push_back(parse(catchBlock,false,infunc));
                 if(start==0)
                 {
                   Final = ast;
                   e = Final;
                 }
                 else
                 {
                   e->childs.push_back(ast);
                   e = e->childs.back();
                 }
                 start=i+1;
                 k = i;
              }
              else if(ast->val=="break" || ast->val=="continue")
              {
                if(!inwhile)
                {
                  parseError("SyntaxError","Error use of break or continue statement is not allowed outside while loop!",false);
                }
                if(start==0)
                {
                  Final = ast;
                  e = Final;
                }
                else
                {
                  e->childs.push_back(ast);
                  e = e->childs.back();
                }
                start = k+1;
              }
              else if(ast->val=="return")
              {
                if(!infunc)
                {
                  parseError("SyntaxError","Error use of return statement outside functon!",false);
                }
                if(start==0)
                {
                  Final = ast;
                  e = Final;
                }
                else
                {
                  e->childs.push_back(ast);
                  e = e->childs.back();
                }
                start = k+1;
              }
              else if(ast->val=="import")
              {
                string str = ast->childs[1]->val;
                if(str.substr(0,4)!="id: ")
                {
                  str = str.substr(8);
                  Node* A;
                  if(std::find(files.begin(),files.end(),str)!=files.end())
                  {
                    A = NewNode("file");
                    A->childs.push_back(NewNode("line "+to_string(line_num)));
                    A->childs.push_back(NewNode(str));
                    A->childs.push_back(NewNode("EOP"));
                  }
                  else
                  {
                    FILE* file = fopen(str.c_str(),"r");
                    if(!file)
                      parseError("ImportError",strerror(errno),false);
                    files.push_back(str);
                    int C = fileTOP;
                    string F = filename;
                    size_t K = line_num;
                    filename = str;
                    line_num = 1;
                    string src;
                    char ch;
                    while((ch = fgetc(file))!=EOF)
                    {
                      src+=ch;
                    }
                //    printf("%s",src.c_str());
                //    printf("----------\n");
                    sources.push_back(src);
                    fileTOP+=1;
                    vector<Token> tokens = generateTokens(src);
                    removeUselessNewlines(tokens);
                    Token t;
                    Token nl;
                    nl.type = NEWLINE_TOKEN;
                    nl.content = "\n";
                    tokens.push_back(nl);
                    t.type = EOP_TOKEN;
                    t.content  = "EOP";
                    tokens.push_back(t);
                    tokens.push_back(nl);
                    A = NewNode("file");
                    A->childs.push_back(NewNode("line "+to_string(K)));
                    A->childs.push_back(NewNode(str));
                    Node* subast = parse(tokens);
                    A->childs.push_back(subast);
                    filename = F;
                    fileTOP = C;
                    line_num = K;

                  }
                  if(start==0)
                  {
                    Final = A;
                    e = A;
                  }
                  else
                  {
                    e->childs.push_back(A);
                    e = A;
                  }
                  start = k+1;
                  deleteAST(ast);
                }
                else
                {
                  if(start==0)
                  {
                    Final = ast;
                    e = Final;
                  }
                  else
                  {
                    e->childs.push_back(ast);
                    e = e->childs.back();
                  }
                  start = k+1;
                }
              }
              else
              {
                if(start==0)
                {
                  Final = ast;
                  e = Final;
                }
                else
                {
                  e->childs.push_back(ast);
                  e = e->childs.back();
                }
                start = k+1;
              }
          }
          k+=1;
      }

      return Final;
  }
};
#endif
