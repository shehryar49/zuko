#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>
#include "json.h"
using namespace std;
enum TokenType
{
    STR=1,
    FLOAT=2,
    NULLVAL=3,
    COMMA=4,
    LCB=5,
    RCB=6,
    LP=7,
    RP=8,
    LSB=9,
    RSB=10,
    COL=11,
    NUM=12,
    BOOL=13

};
const char* StrTokens[] = {
    "STR",
    "FLOAT",
    "NULLVAL",
    "COMMA",
    "LCB",
    "RCB",
    "LP",
    "RP",
    "LSB",
    "RSB",
    "COL",
    "NUM",
    "BOOL"
};
struct Token
{
  TokenType type;
  string content;
  Token(TokenType t,string cont)
  {
    type =t;
    content = cont;
  }
};
vector<Token> tokenize(std::string& str,bool& hadErr,string& msg)
{
  std::vector<Token> tokens;
  hadErr = true;
  size_t l = str.length();
  size_t i = 0;
  while(i<l)
  {
    char c = str[i];
    string cont;
    if(c=='"')
    {
      i+=1;
      int j = i;
      bool escaped = false;
      bool terminated = false;
      while(j<l)
      {
          if(str[j]=='"')
          {
              if(!escaped)
              {
                terminated = true;
                break;
              }
              cont+=str[j];
              escaped = false;
          }
          else if(str[j]=='\\')
          {
            if(escaped)
              cont+="\\";
            else
              escaped = true;
          }
          else
          {
            if(escaped)
            {
              if(str[j]=='n')
                cont+='\n';
              else if(str[j]=='r')
                cont+='\r';
              else if(str[j] == 't')
                cont+='\t';
              else if(str[j]=='b')
                cont+='\b';
              else if(str[j] == 'u')//unicode is not supported atleast not yet
                cont+="\\u";
              else
              {
                msg = (string)"Unknown escape char \\"+str[j];
                return tokens;
              }  
              escaped = false;            
            }
            else
              cont+=str[j];
          }
          j+=1;
      }
      if(!terminated)
      {
          msg = "Non terminated string!";
          return tokens;
      }
      if(escaped)
      {
          msg = "Non terminated escape char!";
          return tokens;
      }
      
      tokens.push_back(Token(STR,cont));
      i = j;
    }
    else if(isdigit(c))
    {
        int j = i;
        bool decimal = false;
        while(j<l)
        {
          if(str[j]=='.' && !decimal)
          {
            decimal = true;
          }
          else if(!isdigit(str[j]))
          {
              break;
          }
          j+=1;
        }
        if(str[j-1]=='.') // floats like 2. are not considered valid it must be 2.0
        {
            msg = "Invalid float format";
            return tokens;
        }
        if(decimal)
          tokens.push_back(Token(FLOAT,str.substr(i,j-i)));                  
        else
          tokens.push_back(Token(NUM,str.substr(i,j-i)));
        i = j;
        continue;
    }
    else if(isalpha(c))
    {
        int j = i;
        while(j<l)
        {
          if(!isalpha(str[j]))
            break;
          j+=1;
        }
        string id = str.substr(i,j-i);
        if(id=="true" || id=="false")
        {
          tokens.push_back(Token(BOOL,id));
        }
        else if(id=="null")
        {
            tokens.push_back(Token(NULLVAL,id));
        }
        else
        {
           msg = "unknown keyword "+id +" "+to_string(i);
           return tokens;
        }
        i = j;
        continue;
    }
    else if(c==',')
    {
        tokens.push_back(Token(COMMA,","));
    }
    else if(c=='[')
    {
        tokens.push_back(Token(LSB,"["));
    }
    else if(c==']')
    {
        tokens.push_back(Token(RSB,"]"));
    }
    else if(c=='{')
    {
        tokens.push_back(Token(LCB,"{"));
    }
    else if(c=='}')
    {
        tokens.push_back(Token(RCB,"}"));
    }
    else if(c==':')
    {
        tokens.push_back(Token(COL,":"));
    }
    else if(c==' ' or c=='\t' or c=='\n' or c=='\r')
    ;
    else
    {
       msg = "Unknown character";
       return tokens;
    }
    i+=1;
  }
  hadErr = false;
  return tokens;
}
bool isValTok(Token t)
{
  if(t.type == NULLVAL || t.type == STR || t.type == FLOAT  || t.type == NUM || t.type==BOOL)
    return true;
  return false;
}
PltObject nil;
PltObject makePbjFromStr(const string& content)
{
  string* p = vm_allocString();
  *p = content;
  PltObject ret;
  ret.type = PLT_STR;
  ret.ptr = (void*)p;
  return ret;
}
PltObject TokToPObj(Token t)
{
  if(t.type == NULLVAL)
    return nil;
  else if(t.type == STR)
    return makePbjFromStr(t.content);
  else if(t.type == FLOAT)
    return PObjFromDouble(atof(t.content.c_str()));
  else if(t.type == NUM)
    return PObjFromInt64(atoll(t.content.c_str()));
  else if(t.type == BOOL)
    return PObjFromBool((t.content == "true") ? true : false);
  return nil;
}

int match_lsb(int start,int end,vector<Token>& tokens)
{
    int I = 0;
    for(int i=start;i<=end;i++)
    {
        if(tokens[i].type==LSB)
            I+=1;
        else if(tokens[i].type==RSB)
        {
            I-=1;
            if(I==0)
              return i;
        }
    }
    return -1;
}
int match_lcb(int start,int end,vector<Token>& tokens)
{
    int I = 0;
    for(int i=start;i<=end;i++)
    {
        if(tokens[i].type==LCB)
            I+=1;
        else if(tokens[i].type==RCB)
        {
            I-=1;
            if(I==0)
              return i;
        }
    }
    return -1;
}
Dictionary* ObjFromTokens(vector<Token>&,int,int,bool&,string&);
PltList* ListFromTokens(std::vector<Token>& tokens,int l,int h,bool& err,string& msg)
{
  err = true;
  PltList* M = vm_allocList();
  if(tokens[l].type != LSB || tokens[h].type!=RSB)
    return M;
  for(int k=l+1;k<h;k+=1)
  {
    if(isValTok(tokens[k]))
    {
      M->push_back(TokToPObj(tokens[k]));
      k+=1;
    }
    else if(tokens[k].type == LCB) //subobject
    {
      int r = match_lcb(k,h-1,tokens);
      if(r == -1)
      {
        msg = "Unmatched curly bracket at "+to_string(k);
        return M;
      }
      PltObject subobj = PObjFromDict(ObjFromTokens(tokens,k,r,err,msg));
      if(err)
        return M;
      err = true;
      k = r+1;
      M->push_back(subobj);
    }
    else if(tokens[k].type == LSB) //list
    {
      int r = match_lsb(k,h-1,tokens);
      if(r == -1)
      {
        msg = "Unmatched square bracket at "+to_string(k);
        return M;
      }
      PltObject sublist = PObjFromList(ListFromTokens(tokens,k,r,err,msg));
      if(err)
      {
        return M;
      }
      err = true;
      k = r+1;
      M->push_back(sublist);
    }
    else
    {
      msg = "Unknown token type ";
      return M;
    }
    if(k != h && tokens[k].type!=COMMA)
    {
      msg = "Expected comma";
      return M;
    }
  }
  err = false;
  return M;
}

Dictionary* ObjFromTokens(std::vector<Token>& tokens,int l,int h,bool& err,string& msg)
{
  err = true;
  Dictionary* M = vm_allocDict();
  if(tokens[l].type != LCB || tokens[h].type!=RCB)
  {
    msg = "SyntaxError";
    return M;
  }
  string key;
  for(int k=l+1;k<h;k+=1)
  {
    if(tokens[k].type!=STR)
    {
      msg = "Non String keys!";
      return M;
    }
    key = tokens[k].content;
    k+=1;
    if(k+1 >= h || tokens[k].type!=COL)
    {
      msg = "Expected ':' after key ";
      return M;
    }
    k+=1;
    //key is at tokens[k-2]
    //values begins at tokens[k]
    if(isValTok(tokens[k]))
    {
      M->emplace(makePbjFromStr(tokens[k-2].content),TokToPObj(tokens[k]));
      k+=1;
    }
    else if(tokens[k].type == LCB) //subobject
    {
      int r = match_lcb(k,h-1,tokens);
      if(r == -1)
      {
        msg = "Unmatched at bracket at "+to_string(k);
        return M;
      }
      PltObject subobj = PObjFromDict(ObjFromTokens(tokens,k,r,err,msg));
      if(err)
        return M;
      err = true;
      k = r+1;    
      M->emplace(makePbjFromStr(key),subobj);
    }
    else if(tokens[k].type == LSB) //list
    {
      int r = match_lsb(k,h-1,tokens);
      if(r == -1)
      {
        msg = "Unmatched square bracket at "+to_string(k);
        return M;
      }
      PltObject sublist = PObjFromList(ListFromTokens(tokens,k,r,err,msg));
      if(err)
        return M;
      err = true;
      k = r+1;
      M->emplace(makePbjFromStr(key),sublist);
    }
    else
    {
      msg = "Unknown value against key "+key;
      return M;
    }
    if(k != h && tokens[k].type!=COMMA)
    {
      msg = "Expected comma after key/value pair";
      return M;
    }
  }
  err = false;
  return M;
}

////
PltObject init()
{
  Module* d = vm_allocModule();
  d->name = "json";
  d->members.emplace("loads",PObjFromFunction("json.loads",&loads));
  return PObjFromModule(d);
}
PltObject loads(PltObject* args,int32_t n)
{
  if(n!=1 || args[0].type!=PLT_STR)
    return Plt_Err(TYPE_ERROR,"String argument required!");
  string& src = *(string*)args[0].ptr;
  bool hadErr;
  string msg;
  vector<Token> tokens = tokenize(src,hadErr,msg);
  if(hadErr)
    return Plt_Err(UNKNOWN_ERROR,"Tokenization failed."+msg);
  Dictionary* m = ObjFromTokens(tokens,0,tokens.size()-1,hadErr,msg);
  if(hadErr)
     return Plt_Err(UNKNOWN_ERROR,"Parsing failed."+msg);
  return PObjFromDict(m);
}
