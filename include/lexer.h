/*MIT License

Copyright (c) 2022 Shahryar Ahmad 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/
#ifndef LEXER_H_
#define LEXER_H_
#include "zuko.h"
using namespace std;
int32_t hexToInt32(const string&);
int64_t hexToInt64(const string&);

const char* keywords[] = {"var","if","else","while","dowhile","import","return","break","continue","function","nil","for","to","dto","step","foreach","namespace","class","private","public","extends","try","catch","throw","yield","as","gc"};
bool isKeyword(string s)
{
  for(size_t k=0;k<sizeof(keywords)/sizeof(char*);k+=1)
  {
    if(s==(string)keywords[k])
    {
      return true;
    }
  }
  return false;
}
string lstrip(string);
void stripNewlines(vector<Token>&);
extern bool REPL_MODE;
void REPL();
const char* getOS();

class Lexer
{
private:
  string filename;
  size_t line_num;
  string source_code;
  void lexErr(string type,string msg)
  {
    fprintf(stderr,"\nFile %s\n",filename.c_str());
    fprintf(stderr,"%s at line %zu\n",type.c_str(),line_num);
    size_t l = 1;
    string line = "";
    size_t k = 0;
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
    fprintf(stderr,"%s\n",lstrip(line).c_str());
    fprintf(stderr,"%s\n",msg.c_str());
    if(REPL_MODE)
      REPL();
   // exit(0);
  }
public:
    static Token resolveMacro(string name)
    {
        Token macro;
        macro.type = NUM_TOKEN;
        if(name == "SEEK_CUR")
          macro.content = to_string(SEEK_CUR);
        else if(name == "SEEK_SET")
          macro.content = to_string(SEEK_SET);
        else if(name == "SEEK_END")
          macro.content = to_string(SEEK_END);
        else if(name == "pi")
        {
          macro.type = FLOAT_TOKEN;
          macro.content = "3.14159";
        }
        else if(name == "e")
        {
          macro.type = FLOAT_TOKEN;
          macro.content = "2.718";
        }
        else if(name == "clocks_per_sec")
        {
          macro.type = FLOAT_TOKEN;
          macro.content=to_string(CLOCKS_PER_SEC);
        }
        else if(name == "FLT_MIN")
        {
          macro.type=FLOAT_TOKEN;
          macro.content = to_string(-DBL_MAX);
        }
        else if(name == "FLT_MAX")
        {
          macro.type=FLOAT_TOKEN;
          macro.content = to_string(DBL_MAX);
        }
        else if(name == "INT_MIN")
          macro.content = to_string(INT_MIN);
        else if(name == "INT_MAX")
          macro.content = to_string(INT_MAX);
        else if(name == "INT64_MIN")
          macro.content = to_string(LLONG_MIN);
        else if(name == "INT64_MAX")        
          macro.content = to_string(LLONG_MAX);
        else if(name == "os")
        {
          macro.type = STRING_TOKEN;
          macro.content = getOS();
        }
        else 
          macro.type = EOP_TOKEN;//to indicate macro not found
        return macro;

    }
    vector<Token> generateTokens(string fname,const string& s)
    {
        //fname is current filename
        //it must always be present in files vector
        line_num = 1;
        filename = fname;
        source_code = s;
        size_t k = 0;
        size_t srcLen = s.length();
        char c;
        vector<Token> tokenlist;
        int ln = 1;
        while(k<srcLen)
        {
            c = s[k];
            if(c=='"')
            {
                size_t j = k+1;
                string t;
                bool match = false;
                bool escaped = false;
                int LN = ln;
                while(j<srcLen)
                {
                    if(s[j]=='"')
                    {
                        if(escaped)
                          escaped = false;
                        else
                        {
                          match = true;
                          break;
                        }
                        t+=s[j];
                    }
                    else if(s[j]=='\\' && !escaped)
                    {
                        escaped = true;
                        t+=s[j];
                    }
                    else if(s[j]=='\n')
                    {
                        t+=s[j];
                        ln+=1;
                    }
                    else
                    {
                        t+=s[j];
                        if(escaped)
                        {
                            escaped=false;
                        }
                    }
                    j+=1;
                }
                if(!match)
                {
                    line_num = LN;
                    lexErr("SyntaxError","Error expected a '\"' at the end!");
                    return {};
                }
                Token i;
                line_num = ln;
                bool hadErr = false;
                i.content = addlnbreaks(t,hadErr);
                if(hadErr)
                {
                    line_num = ln;
                    lexErr("SyntaxError",i.content);
                    return {};
                }
                i.type = STRING_TOKEN;
                i.ln = ln;
                tokenlist.push_back(i);
                k = j;
            }
            else if(c=='@')
            {
                if(k==srcLen-1)
                {
                    line_num = ln;
                    lexErr("SyntaxError","Invalid macro name");
                    return {};
                }
                size_t j = k+1;
                string t = "";
                while(j<srcLen)
                {

                    if(!isalpha(s[j]) && !isdigit(s[j]) && s[j]!='_')
                    break;
                    if(j==k+1 && isdigit(s[j]))
                    {
                        line_num = ln;
                        lexErr("SyntaxError","Invalid macro name");
                        return {};
                    }
                    t+=s[j];
                    j+=1;
                }
                Token tok;
                if(t == "file")
                {
                  tok.type = TokenType::STRING_TOKEN;
                  tok.content = filename;
                }
                else if(t == "lineno")
                {
                  tok.type = TokenType::NUM_TOKEN;
                  tok.content = to_string(ln);
                }
                else
                {
                  tok = resolveMacro(t);
                  if(tok.type == EOP_TOKEN)
                  {
                    line_num = ln;
                    lexErr("NameError","Unknown macro "+t);
                    return {};
                  }
                }
                tokenlist.push_back(tok);
                k = j;
                continue;
            }
            else if(c=='#')
            {
                bool multiline = false;
                bool foundEnd = false;
                if(k+1 < srcLen && s[k+1]=='-')
                  multiline = true;
                size_t j=k+1;
                size_t orgLn = ln;
                for(;j<srcLen;j+=1)
                {
                    if(!multiline && s[j]=='\n')
                    {
                        break;
                    }
                    else if(multiline && s[j] == '-')
                    {
                        if(j+1 < srcLen && s[j+1]=='#')
                        {
                            foundEnd = true;
                            j+=2;
                            break;
                        }
                    }
                    else if(s[j] == '\n')
                      ln++;
                }
                if(multiline && !foundEnd)
                {
                  line_num = orgLn;
                  lexErr("SyntaxError","Unable to find the end of multiline comment!");
                  return {};
                }
                k = j-1;
            }
            else if(isdigit(c))
            {
                //hex literal
                if(c=='0' && k!=srcLen-1 && s[k+1]=='x')
                {

                    k+=1;
                    size_t j = k+1;
                    string b;
                    Token i;
                    while(j<srcLen)
                    {
                        c = s[j];
                        if(c>='0' && c<='9')
                            b+=c;
                        else if(c>='a' && c<='z')
                          b+=c;
                        else
                          break;
                        j+=1;

                    }
                    if(b.length() == 1 || b.length() == 2) //byte
                    {
                       if(b.length() == 1)
                         b = "0" + b;
                       i.type = BYTE_TOKEN;
                       i.ln = ln;
                       i.content = b;
                       tokenlist.push_back(i);
                    }
                    else if(b.length() >= 3 && b.length()<=8)//int32
                    {
                       i.type = NUM_TOKEN;
                       i.ln = ln;
                       i.content = to_string(hexToInt32(b));
                       tokenlist.push_back(i);
                    }
                    else if(b.length()>8 &&  b.length() <= 16)//int64
                    {
                       i.type = NUM_TOKEN;
                       i.ln = ln;
                       i.content = to_string(hexToInt64(b));
                       tokenlist.push_back(i);
                    }
                    else
                    {
                        line_num = ln;
                        lexErr("SyntaxError","Invalid Syntax");
                        return {};
                    }
                    k = j;
                    continue;
                }
                size_t j = k+1;
                string t;
                t+=c;
                bool decimal = false;
                bool exp = false;

                bool expSign = false;
                while(j<srcLen)
                {
                    if(!isdigit(s[j]) && s[j]!='.' && s[j]!='e' && s[j]!='-' && s[j]!='+')
                    {
                        j = j-1;
                        break;
                    }
                    if(s[j]=='.')
                    {
                        if(decimal || exp)
                        {
                            j = j-1;
                            break;
                        }
                        decimal = true;
                    }
                    else if(s[j]=='e')
                    {
                        if(exp)
                        {
                            j = j-1;
                            break;
                        }
                        exp = true;
                    }
                    else if(s[j]=='+' || s[j]=='-')
                    {
                        if(expSign || !exp)
                        {
                            j = j-1;
                            break;
                        }
                        expSign = true;
                    }
                    t+=s[j];
                    j+=1;
                }
                if(t[t.size()-1]=='e')
                {
                    t = t.substr(0,t.length()-1);
                    j-=1;
                }
                Token i;
                i.content = t;
                if(!decimal && !exp)
                i.type = NUM_TOKEN;
                else
                i.type = FLOAT_TOKEN;
                i.ln = ln;
                tokenlist.push_back(i);
            k = j;
            }
            else if(c=='>' || c=='<')
            {
                if(k+1 < srcLen && s[k+1]=='=')
                {
                    Token i;
                    i.type = TokenType::OP_TOKEN;
                    i.content+=c;
                    i.content+="=";
                    i.ln = ln;
                    tokenlist.push_back(i);
                    k = k+1;
                }
                else if(c=='<' && s[k+1]=='<')
                {
                    Token i;
                    i.type=TokenType::OP_TOKEN;
                    i.content ="<<";
                    tokenlist.push_back(i);
                    k+=1;
                }
                else if(c=='>' && k+1 < srcLen && s[k+1]=='>')
                {
                    Token i;
                    i.type=TokenType::OP_TOKEN;
                    i.content =">>";
                    tokenlist.push_back(i);
                    k+=1;
                }
                else
                {
                    Token i;
                    i.type = TokenType::OP_TOKEN;
                    i.content+=c;
                    i.ln = ln;
                    tokenlist.push_back(i);

                }
            }
            else if(c=='.')
            {
                Token i;
                i.type = TokenType::OP_TOKEN;
                i.content+=c;
                i.ln = ln;
                tokenlist.push_back(i);
            
            }
            else if(c=='+' || c=='-')
            {
                if(k!=srcLen-1)
                {
                    if(s[k+1]=='=')
                    {
                    Token i;
                    i.ln = ln;
                    i.content += c;
                    i.content+="=";
                    i.type = TokenType::OP_TOKEN;
                    tokenlist.push_back(i);
                    k+=2;
                    continue;
                    }
                }
                Token i;
                i.ln = ln;
                i.content += c;
                i.type = TokenType::OP_TOKEN;
                tokenlist.push_back(i);
            }
            else if(c=='/' || c=='*' || c=='%' || c=='^' || c=='&' || c=='|' || c=='~')
            {

                if(k!=srcLen-1)
                {
                    if(s[k+1]=='=')
                    {
                    Token i;
                    i.ln = ln;
                    i.content += c;
                    i.content+="=";
                    i.type = TokenType::OP_TOKEN;
                    tokenlist.push_back(i);
                    // = "TokenType::OP_TOKEN";
                    k+=2;
                    continue;
                    }
                }
                Token i;
                i.ln = ln;
                i.content += c;
                i.type = TokenType::OP_TOKEN;
                tokenlist.push_back(i);
                // = "TokenType::OP_TOKEN";


            }
            else if(c=='(')
            {
                Token i;
                i.content += c;
                i.type = TokenType::LParen_TOKEN;
                i.ln = ln;
                tokenlist.push_back(i);
            }
            else if(c==')')
            {
                Token i;
                i.content += c;
                i.type = TokenType::RParen_TOKEN;
                            i.ln = ln;
                tokenlist.push_back(i);
            }
            else if(c=='!')
            {
                if(k!=srcLen-1)
                {
                    if(s[k+1]=='=')
                    {
                        Token i;
                        i.content = "!=";
                        i.type = TokenType::OP_TOKEN;
                        i.ln = ln;
                        k+=1;
                        tokenlist.push_back(i);
                    }
                    else
                    {
                        Token i;
                        i.content = "!";
                        i.type = TokenType::OP_TOKEN;
                        i.ln = ln;
                        tokenlist.push_back(i);
                    }
                }
                else
                {
                    Token i;
                    i.content = "!";
                    i.type = TokenType::OP_TOKEN;
                    i.ln = ln;
                    tokenlist.push_back(i);
                }
            }
            else if(c=='=')
            {
                if(s[k+1]=='=')
                {
                Token i;
                i.content = "==";
                i.type = TokenType::OP_TOKEN;
                i.ln = ln;
                k+=1;
                tokenlist.push_back(i);
                }
                else
                {
                Token i;
                i.content = "=";
                i.type = TokenType::OP_TOKEN;
                i.ln = ln;
                tokenlist.push_back(i);
                }
            }

            else if(c=='[')
            {
                Token i;
                i.content = "[";
                i.type = BEGIN_LIST_TOKEN;
                i.ln = ln;
                tokenlist.push_back(i);
            }
            else if(c==']')
            {

                    Token i;
                    i.content = "]";
                    i.type = END_LIST_TOKEN;
                            i.ln = ln;
                    tokenlist.push_back(i);
            }
            else if(c=='{')
            {

                Token i;
                i.content = "{";
                i.type = L_CURLY_BRACKET_TOKEN;
                i.ln = ln;
                tokenlist.push_back(i);
            }
            else if(c=='}')
            {
                Token i;
                i.content = "}";
                i.type = R_CURLY_BRACKET_TOKEN;
                i.ln = ln;
                tokenlist.push_back(i);
            }
            else if(isalpha(c) || c=='_')
            {
                size_t j = k+1;
                string t;
                t+=c;
                while(j<srcLen)
                {
                    if((j!=srcLen-1 && s[j]==':' && s[j+1]==':'))
                    {
                        t+="::";
                        j+=2;
                    }
                    else if(!isalpha(s[j]) && !isdigit(s[j]) && s[j]!='_')
                    {
                        j = j-1;
                        break;
                    }
                    else
                    {
                        t+=s[j];
                        j+=1;
                    }
                }

                Token i;
                if(isKeyword(t))
                {
                    if(t=="if" && k!=0 && tokenlist.size()!=0)
                    {
                        if(tokenlist[tokenlist.size()-1].type == KEYWORD_TOKEN && tokenlist[tokenlist.size()-1].content=="else")
                        {
                            tokenlist[tokenlist.size()-1].content+=" if";
                            k = j+1;
                            continue;
                        }
                    }
                    i.type = KEYWORD_TOKEN;
                    i.content = t;
                    i.ln = ln;
                }
                else if(t=="or" || t=="and" || t=="is")
                {
                    i.content = t;
                    i.type = TokenType::OP_TOKEN;
                    i.ln = ln;
                }
                else if(t=="true" || t=="false")
                {
                    i.content = t;
                    i.type = BOOL_TOKEN;
                    i.ln = ln;
                }
                else
                {
                    i.type = ID_TOKEN;
                    i.content = t;
                    i.ln = ln;
                }
                tokenlist.push_back(i);
                k = j;
            }
            else if(c==',')
            {
                Token i;
                i.type = COMMA_TOKEN;
                i.content = ",";
                i.ln = ln;
                tokenlist.push_back(i);
            }
            else if(c==':')
            {
                Token i;
                i.type = COLON_TOKEN;
                i.content = ":";
                i.ln = ln;
                tokenlist.push_back(i);
            }
            else if(c=='\n' )
            {
                Token i;
                i.content = "\n";
                i.type = NEWLINE_TOKEN;;
                i.ln = -1;
                tokenlist.push_back(i);
                ln+=1;
            }
            else if(c=='\t' )
            {

                //do nothing
            }
            else if(c==' ' )
            {
                //do nothing
            }
            else if(c=='\r')
            {

            }
            else
            {
                //error;
                line_num = ln;
                lexErr("SyntaxError",(string)"Illegal character '"+c+"'");
                return {};
            }
            k+=1;
        }
        stripNewlines(tokenlist);//removes newlines start and end of token list
        if (tokenlist.size() == 0)
            return tokenlist;
        Token t;
        t.type = NEWLINE_TOKEN;
        t.content = "\n";
        tokenlist.push_back(t);
        t.content = "EOP";
        t.type = EOP_TOKEN;
        tokenlist.push_back(t);
        t.type = NEWLINE_TOKEN;
        t.content = "\n";
        tokenlist.push_back(t);
        return tokenlist;
    }
};
#endif // LEXER_H_
