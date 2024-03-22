#include "lexer.h"
#include "parser.h"
#include "token.h"
#include "utility.h"
#include <cctype>
#include <float.h>
#include <time.h>
#include <limits.h>
extern bool REPL_MODE;
void REPL();

const char* Lexer::keywords[] = {"var","if","else","while","dowhile","import","return","break","continue","function","nil","for","to","dto","step","foreach","namespace","class","private","public","extends","try","catch","throw","yield","as","gc"};
bool Lexer::isKeyword(string s)
{
  for(size_t k=0;k<sizeof(keywords)/sizeof(char*);k+=1)
  {
    if(s==(string)keywords[k])
      return true;
  }
  return false;
}
void Lexer::lexErr(string type,string msg)
{
    hadErr = true;
    errmsg = msg;
    if(!printErr)
      return;
    fprintf(stderr,"\nFile %s\n",filename.c_str());
    fprintf(stderr,"%s at line %zu\n",type.c_str(),line_num);
    size_t l = 1;
    string line = "";
    size_t k = 0;
    while(l<=line_num)
    {
        if(source_code[k]=='\n')
            l+=1;
        else if(l==line_num)
            line+=source_code[k];
        k+=1;
        if(k>=source_code.length())
          break;      
    }
    fprintf(stderr,"%s\n",lstrip(line).c_str());
    fprintf(stderr,"%s\n",msg.c_str());
    if(REPL_MODE)
      REPL();
}
Token Lexer::resolveMacro(const string& name)
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
void Lexer::str(size_t& k,vector<Token>& tokenlist)
{
    size_t j = k+1;
    string t;
    bool match = false;
    bool escaped = false;
    int LN = line_num;
    while(j<srcLen)
    {
        if(source_code[j]=='"')
        {
            if(!escaped)
            {
                match = true;
                break;
            }
            escaped = false;
            t+=source_code[j];
        }
        else if(source_code[j]=='\n')
        {
            t+=source_code[j];
            line_num+=1;
        }
        else if(source_code[j]=='\\' && !escaped)
            escaped = true;
        else if(source_code[j] == '\\' && escaped)
        {
            t += '\\';
            escaped = false;
        }
        else if(source_code[j] == 'a' && escaped)
        {
            t += '\a';
            escaped = false;
        }
        else if(source_code[j] == 'r' && escaped)
        {
            t += '\r';
            escaped = false;
        }
        else if(source_code[j] == 'n' && escaped)
        {
            t += '\n';
            escaped = false;
        }
        else if(source_code[j] == 't' && escaped)
        {
            t += '\t';
            escaped = false;
        }
        else if(source_code[j] == 'b' && escaped)
        {
            t += '\b';
            escaped = false;
        }
        else if(source_code[j] == 'v' && escaped)
        {
            t += '\v';
            escaped = false;
        }
        else if(source_code[j] == 'x' && escaped)
        {
            if(j + 2 >= srcLen)
            {
                lexErr("SyntaxError","Expected 2 hex digits after '\\x'");
                return;
            }
            if(!isalpha(source_code[j+1]) && !isdigit(source_code[j+1]))
            {
                lexErr("SyntaxError","Expected 2 hex digits after '\\x'");
                return;
            }
            if(!isalpha(source_code[j+2]) && !isdigit(source_code[j+2]))
            {
                lexErr("SyntaxError","Expected 2 hex digits after '\\x'");
                return;
            }
            string tmp = source_code.substr(j+1,2);
            unsigned char ch = tobyte(tmp);
            t+=ch;
            j += 2;
            escaped = false;
        }
        
        else
        {
            if(escaped) // unknown escape sequence
            {
                lexErr("SyntaxError","Unknown escape character in string!");
                return;
            }
            t+=source_code[j];
        }
        j+=1;
    }
    if(!match)
    {
        line_num = LN;
        lexErr("SyntaxError","Error expected a '\"' at the end!");
        return;
    }
    if(escaped)
    {
        lexErr("SyntaxError","String has non terminated escape sequence!");
        return;
    }

    tokenlist.push_back((Token(STRING_TOKEN,t,line_num)));
    k = j;
}
void Lexer::macro(size_t& k,vector<Token>& tokenlist)
{
    if(k==srcLen-1)
    {
        lexErr("SyntaxError","Invalid macro name");
        return;
    }
    size_t j = k+1;
    string t = "";
    while(j<srcLen)
    {

        if(!isalpha(source_code[j]) && !isdigit(source_code[j]) && source_code[j]!='_')
            break;
        if(j==k+1 && isdigit(source_code[j]))
        {
            k = srcLen - 1;
            lexErr("SyntaxError","Invalid macro name");
            return;
        }
        t+=source_code[j];
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
        tok.content = to_string(line_num);
    }
    else
    {
        tok = resolveMacro(t);
        if(tok.type == EOP_TOKEN)
        {
            k = srcLen - 1;
            lexErr("NameError","Unknown macro "+t);
            return;
        }
    }
    tokenlist.push_back(tok);
    k = j;
}
void Lexer::comment(size_t& k,vector<Token>& tokenlist)
{
    bool multiline = false;
    bool foundEnd = false;
    if(k+1 < srcLen && source_code[k+1]=='-')
        multiline = true;
    size_t j=k+1;
    size_t orgLn = line_num;
    for(;j<srcLen;j+=1)
    {
        if(!multiline && source_code[j]=='\n')
        {
            break;
        }
        else if(multiline && source_code[j] == '-')
        {
            if(j+1 < srcLen && source_code[j+1]=='#')
            {
                foundEnd = true;
                j+=2;
                break;
            }
        }
        else if(source_code[j] == '\n')
            line_num++;
    }
    if(multiline && !foundEnd)
    {
        k = srcLen - 1;
        line_num = orgLn;
        lexErr("SyntaxError","Unable to find the end of multiline comment!");
        return;
    }
    k = j-1;
}
void Lexer::numeric(size_t& k,vector<Token>& tokenlist)
{
    //hex literal
    char c = source_code[k];
    if(c=='0' && k!=srcLen-1 && source_code[k+1]=='x')
    {

        k+=1;
        size_t j = k+1;
        string b;
        Token i;
        while(j<srcLen)
        {
            c = source_code[j];
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
            i.ln = line_num;
            i.content = b;
            tokenlist.push_back(i);
        }
        else if(b.length() >= 3 && b.length()<=8)//int32
        {
            i.type = NUM_TOKEN;
            i.ln = line_num;
            i.content = to_string(hexToInt32(b));
            tokenlist.push_back(i);
        }
        else if(b.length()>8 &&  b.length() <= 16)//int64
        {
            i.type = NUM_TOKEN;
            i.ln = line_num;
            i.content = to_string(hexToInt64(b));
            tokenlist.push_back(i);
        }
        else
        {
            lexErr("SyntaxError","Invalid Syntax");
            return;
        }
        k = j-1;
        return;
    }
    size_t j = k+1;
    string t;
    t+=c;
    bool decimal = false;
    bool exp = false;

    bool expSign = false;
    while(j<srcLen)
    {
        if(!isdigit(source_code[j]) && source_code[j]!='.' && source_code[j]!='e' && source_code[j]!='-' && source_code[j]!='+')
        {
            j = j-1;
            break;
        }
        if(source_code[j]=='.')
        {
            if(decimal || exp)
            {
                j = j-1;
                break;
            }
            decimal = true;
        }
        else if(source_code[j]=='e')
        {
            if(exp)
            {
                j = j-1;
                break;
            }
            exp = true;
        }
        else if(source_code[j]=='+' || source_code[j]=='-')
        {
            if(expSign || !exp)
            {
                j = j-1;
                break;
            }
            expSign = true;
        }
        t+=source_code[j];
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
    i.ln = line_num;
    tokenlist.push_back(i);
    k = j;
}
void Lexer::id(size_t& k,vector<Token>& tokenlist)
{
    size_t j = k+1;
    string t;
    t+=source_code[k];
    while(j<srcLen)
    {
        if((j!=srcLen-1 && source_code[j]==':' && source_code[j+1]==':'))
        {
            t+="::";
            j+=2;
        }
        else if(!isalpha(source_code[j]) && !isdigit(source_code[j]) && source_code[j]!='_')
        {
            j = j-1;
            break;
        }
        else
        {
            t+=source_code[j];
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
                k = j;
                return;
            }
        }
        i.type = KEYWORD_TOKEN;
        i.content = t;
        i.ln = line_num;
    }
    else if(t=="or" || t=="and" || t=="is")
    {
        i.content = t;
        i.type = TokenType::OP_TOKEN;
        i.ln = line_num;
    }
    else if(t=="true" || t=="false")
    {
        i.content = t;
        i.type = BOOL_TOKEN;
        i.ln = line_num;
    }
    else
    {
        i.type = ID_TOKEN;
        i.content = t;
        i.ln = line_num;
    }
    tokenlist.push_back(i);
    k = j;
}
vector<Token> Lexer::generateTokens(string fname,const string& s,bool printErr)
{
    //fname is the filename
    //s is the file source

    line_num = 1;
    filename = fname;
    source_code = s;
    srcLen = source_code.length();
    this->printErr = printErr;
    size_t k = 0;
    char c;
    string tmp;
    vector<Token> tokenlist;
    while(!hadErr && k<srcLen)
    {
        c = s[k];
        if(c=='"')
            str(k, tokenlist);
        else if(c=='@')
        {
            macro(k, tokenlist);
            continue;
        }
        else if(c=='#')
            comment(k, tokenlist);
        else if(isdigit(c))
            numeric(k,tokenlist);
        else if(c=='>' || c=='<')
        {
            if(k+1 < srcLen && s[k+1]=='=')
            {
                tmp = c;
                tmp += '=';
                tokenlist.push_back(Token(OP_TOKEN,tmp,line_num));
                k = k+1;
            }
            else if(c=='<' && k+1<srcLen &&  s[k+1]=='<')
            {
                tokenlist.push_back(Token(OP_TOKEN,"<<",line_num));
                k+=1;
            }
            else if(c=='>' && k+1 < srcLen && s[k+1]=='>')
            {
                tokenlist.push_back(Token(OP_TOKEN,">>",line_num));
                k+=1;
            }
            else
                tokenlist.push_back(Token(OP_TOKEN,c,line_num));
        }
        else if(c=='.')
            tokenlist.push_back(Token(OP_TOKEN,'.',line_num));        
        else if(c=='+' || c=='-')
        {
            if(k!=srcLen-1 && s[k+1]=='=')
            {
                tmp = c;
                tmp +="=";
                tokenlist.push_back(Token(OP_TOKEN,tmp,line_num));
                k+=2;
                continue;
            }
            tokenlist.push_back(Token(OP_TOKEN,c,line_num));
        }
        else if(c=='/' || c=='*' || c=='%' || c=='^' || c=='&' || c=='|' || c=='~')
        {
            if(k!=srcLen-1 && s[k+1] == '=')
            {
                tmp = c;
                tmp+="=";
                tokenlist.push_back(Token(OP_TOKEN,tmp,line_num));
                k+=2;
                continue;
            }
            tokenlist.push_back(Token(OP_TOKEN,c,line_num));
        }
        else if(c=='!')
        {
            if(k!=srcLen-1 && s[k+1] == '=')
            {
                k+=1;
                tokenlist.push_back(Token(OP_TOKEN,"!=",line_num));
            }
            else
                tokenlist.push_back(Token(OP_TOKEN,"!",line_num));
        }
        else if(c=='=')
        {
            if(k!=srcLen-1 && s[k+1]=='=')
            {
                tokenlist.push_back(Token(OP_TOKEN,"==",line_num));
                k+=1;
            }
            else
                tokenlist.push_back(Token(OP_TOKEN,"=",line_num));  
        }
        else if(c=='(')
            tokenlist.push_back(Token(LParen_TOKEN,c,line_num));
        else if(c==')')
            tokenlist.push_back(Token(RParen_TOKEN,c,line_num));
        else if(c=='[')
            tokenlist.push_back(Token(BEGIN_LIST_TOKEN,"[",line_num));
        else if(c==']')
            tokenlist.push_back(Token(END_LIST_TOKEN,"]",line_num));
        else if(c=='{')
            tokenlist.push_back(Token(L_CURLY_BRACKET_TOKEN,"{",line_num));
        else if(c=='}')
            tokenlist.push_back(Token(R_CURLY_BRACKET_TOKEN,"}",line_num));
        else if(isalpha(c) || c=='_')
            id(k,tokenlist);
        else if(c==',')
            tokenlist.push_back(Token(COMMA_TOKEN,",",line_num));
        else if(c==':')
            tokenlist.push_back(Token(COLON_TOKEN,":",line_num));
        else if(c=='\n' )
        {
            tokenlist.push_back(Token(NEWLINE_TOKEN,"\n",line_num));
            line_num++;
        }
        else if(c=='\t' || c==' ' || c=='\r')
           ; //do nothing
        else
        {
            //error;
            lexErr("SyntaxError",(string)"Illegal character '"+c+"'");
            return {};
        }
        k+=1;
    }
    if(hadErr)
      return {}; // empty tokenlist indicates error
    stripNewlines(tokenlist);//removes newlines start and end of token list
    if (tokenlist.size() == 0)
      return {Token(EOP_TOKEN,"",0)}; // empty program
    
    //the following is just the way, parser expects it to be 
    tokenlist.push_back(Token(NEWLINE_TOKEN,"\n",0));
    tokenlist.push_back(Token(EOP_TOKEN,"EOP",0));
    tokenlist.push_back(Token(NEWLINE_TOKEN,"\n",0));

    return tokenlist;
}