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
  int line_num = 1;
  int line_begin = 0;
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
           msg = "unknown keyword "+id +" at "+to_string(line_num)+":"+to_string(i-line_begin);
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
    else if(c == '\n')
    {
      line_num++;
      line_begin = i+1;
    }
    else if(c==' ' || c=='\t'  || c=='\r')
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
ZObject nil;

ZObject TokToPObj(Token t)
{
  if(t.type == NULLVAL)
    return nil;
  else if(t.type == STR)
    return ZObjFromStr(t.content.c_str());
  else if(t.type == FLOAT)
    return ZObjFromDouble(atof(t.content.c_str()));
  else if(t.type == NUM)
    return ZObjFromInt64(atoll(t.content.c_str()));
  else if(t.type == BOOL)
    return ZObjFromBool((t.content == "true") ? true : false);
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
ZDict* ObjFromTokens(vector<Token>&,int,int,bool&,string&);

ZList* ListFromTokens(std::vector<Token>& tokens,int l,int h,bool& err,string& msg)
{
  err = true;
  ZList* M = vm_allocList();
  if(tokens[l].type != LSB || tokens[h].type!=RSB)
    return M;
  for(int k=l+1;k<h;k+=1)
  {
    if(isValTok(tokens[k]))
    {
      ZList_push(M,TokToPObj(tokens[k]));
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
      ZObject subobj = ZObjFromDict(ObjFromTokens(tokens,k,r,err,msg));
      if(err)
        return M;
      err = true;
      k = r+1;
      ZList_push(M,subobj);
    }
    else if(tokens[k].type == LSB) //list
    {
      int r = match_lsb(k,h-1,tokens);
      if(r == -1)
      {
        msg = "Unmatched square bracket at "+to_string(k);
        return M;
      }
      ZObject sublist = ZObjFromList(ListFromTokens(tokens,k,r,err,msg));
      if(err)
      {
        return M;
      }
      err = true;
      k = r+1;
      ZList_push(M,sublist);
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

ZDict* ObjFromTokens(std::vector<Token>& tokens,int l,int h,bool& err,string& msg)
{
  err = true;
  ZDict* M = vm_allocDict();
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
      ZDict_emplace(M,ZObjFromStr(tokens[k-2].content.c_str()),TokToPObj(tokens[k]));
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
      ZObject subobj = ZObjFromDict(ObjFromTokens(tokens,k,r,err,msg));
      if(err)
        return M;
      err = true;
      k = r+1;    
      ZDict_emplace(M,ZObjFromStr(key.c_str()),subobj);
    }
    else if(tokens[k].type == LSB) //list
    {
      int r = match_lsb(k,h-1,tokens);
      if(r == -1)
      {
        msg = "Unmatched square bracket at "+to_string(k);
        return M;
      }
      ZObject sublist = ZObjFromList(ListFromTokens(tokens,k,r,err,msg));
      if(err)
        return M;
      err = true;
      k = r+1;
      ZDict_emplace(M,ZObjFromStr(key.c_str()),sublist);
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
bool dumperror;
std::string dumperrmsg;
void PObjToStr(ZObject p,std::string& res,std::unordered_map<void*,bool>& seen)
{
  if(p.type == Z_INT)
    res+= to_string(p.i);
  else if(p.type == Z_INT64)
    res+= to_string(p.l);
  else if(p.type == Z_FLOAT)
    res+= to_string(p.f);
  else if(p.type == Z_STR)
    res+= "\""+(*(string*)p.ptr)+"\"";
  else if(p.type == Z_BOOL)
    res += (p.i) ? "true" : "false";
  else if(p.type == Z_NIL)
    res+="null";
  else if(p.type == Z_LIST)
  {
    if(seen.find(p.ptr) != seen.end())
      res+="[...]";
    else
    {
      res+="[";
      seen.emplace(p.ptr,true);
      ZList* l = (ZList*)p.ptr;
      size_t len = l->size;
      size_t k = 0;
      for(size_t idx=0;idx<l->size;idx++)
      {
        auto e = l->arr[idx];
        PObjToStr(e,res,seen);
        if(dumperror)
          return;
        if(k++ != len-1)
          res+=",";
      }
      res += "]";
    }
  }
  else if(p.type == Z_DICT)
  {
    if(seen.find(p.ptr) != seen.end())
      res+="{...}";
    else
    {
      res+="{";
      seen.emplace(p.ptr,true);
      ZDict* d = (ZDict*)p.ptr;
      size_t l = d->size;
      size_t k = 0;
      for(size_t idx = 0; idx < d->capacity;idx++)
      {
        if(d->table[idx].stat != OCCUPIED)
          continue;
        auto& e = d->table[idx];
        if(e.key.type != Z_STR) //json constraint fail
        {
          dumperror = true;
          dumperrmsg = "Dictionary key not a string!";
          return;
        }
        res += "\"" + (string)(((ZStr*)e.key.ptr))->val + "\"";
        res += ": ";
        PObjToStr(e.val,res,seen);
        if(dumperror)
          return;
        if(k++ != l-1)
          res+=",";
      }
      res += "}";
    }
  }
}
///
Klass* parseError;
Klass* tokenizeError;
////
ZObject init()
{
  nil.type = Z_NIL;
  //initialize custom errors
  parseError = makeDerivedKlass(Error);
  parseError->name = "ParseError";
  tokenizeError = makeDerivedKlass(Error);
  tokenizeError->name = "TokenizeError";
  vm_markImportant(parseError);
  vm_markImportant(tokenizeError);
  //
  Module* d = vm_allocModule();
  d->name = "json";
  Module_addNativeFun(d,"loads",&loads);
  Module_addNativeFun(d,"dumps",&dumps);
  return ZObjFromModule(d);
}
ZObject loads(ZObject* args,int32_t n)
{
  if(n!=1 || args[0].type!=Z_STR)
    return Z_Err(TypeError,"String argument required!");
  string& src = *(string*)args[0].ptr;
  bool hadErr;
  string msg;
  vector<Token> tokens = tokenize(src,hadErr,msg);
  if(hadErr)
  {
    msg = "Tokenization failed. "+msg; 
    return Z_Err(tokenizeError,msg.c_str());
  }
  if(tokens.size() == 0)
    return Z_Err(parseError,"Empty string!");
  ZDict* m = ObjFromTokens(tokens,0,tokens.size()-1,hadErr,msg);
  if(hadErr)
  {
     msg = "Parsing failed. "+msg;
     return Z_Err(parseError,msg.c_str());
  }
  return ZObjFromDict(m);
}
ZObject dumps(ZObject* args,int32_t n)
{
  if(n!=1)
    return Z_Err(ArgumentError,"1 argument needed!");
  if(args[0].type != Z_DICT)
    return Z_Err(TypeError,"Argument must be a dictionary!");
  dumperrmsg = "";
  dumperror = false;
  string res;
  std::unordered_map<void*,bool> seen;
  PObjToStr(args[0],res,seen);
  if(dumperror)
    return Z_Err(Error,dumperrmsg.c_str());
  return ZObjFromStr(res.c_str());
}
void unload()
{
  vm_unmarkImportant(parseError);
  vm_unmarkImportant(tokenizeError);
}