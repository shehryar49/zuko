#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "token-vector.h"
#include "token.h"
#include <float.h>
#include <time.h>
#include <limits.h>
#include "dyn-str.h"

extern bool REPL_MODE;
void REPL();

const char* keywords[] = {"var","if","else","while","dowhile","import","return","break","continue","function","nil","for","to","dto","step","foreach","namespace","class","private","public","extends","try","catch","throw","yield","as","gc"};
bool isKeyword(const char* s)
{
  for(size_t k=0;k<sizeof(keywords)/sizeof(char*);k+=1)
  {
    if(strcmp(s,keywords[k]) == 0)
      return true;
  }
  return false;
}
static unsigned char tobyte(const char* s)
{
    //s.length() is always 2

    char x = tolower(s[0]);
    char y = tolower(s[1]);
    unsigned char b = 0;
    b = (isdigit(y)) ? y-48 : y-87;
    b += (isdigit(x)) ? (x-48)<<4 : (x-87)<<4;
    return b;
}
static char* clone_str(const char* str)
{
    size_t len = strlen(str);
    char* n = malloc(sizeof(char)*(len+1));
    strcpy(n,str); //it is safe so STFU
    return n;
}
char* char_to_str(char ch)
{
    char tmp[2] = {ch,0};
    return clone_str(tmp);
}
char* make_substr(const char* s,size_t strlen,size_t idx,size_t len)
{
    char* d = (char*)malloc(sizeof(char)*(strlen+1));
    memcpy(d,s+idx,len);
    d[len] = 0;
    return d;
}
void lexErr(lexer* ctx,const char* type,const char* msg)
{
    ctx->hadErr = true;
    ctx->errmsg = msg;
    if(!ctx->printErr)
      return;
    fprintf(stderr,"\nFile %s\n",ctx->filename);
    fprintf(stderr,"%s at line %zu\n",type,ctx->line_num);
    size_t l = 1;
    size_t k = 0;
    while(ctx->source_code[k]!=0 && l<=ctx->line_num)
    {
        if(ctx->source_code[k]=='\n')
            l+=1;
        else if(l==ctx->line_num)
            fputc(ctx->source_code[k],stderr);
        k+=1;
    }
    fprintf(stderr,"%s\n",msg);
    //if(REPL_MODE) IMPORTANT: FIX later
    //  REPL();
}
char* int64_to_string(int64_t x)
{
    char buffer[50];
    snprintf(buffer,50,"%zd",x);
    return clone_str(buffer);
}
char* double_to_string(double x)
{
    char buffer[50];
    snprintf(buffer,50,"%f",x);
    return clone_str(buffer);
}
static int32_t hex_to_int32(const char* s)
{
    int32_t res = 0;
    int32_t p = 1;
    int32_t len = strlen(s);
    for(int32_t i=len-1;i>=0;i--)
    {
        if(s[i] >= '0' && s[i]<='9')
        {
            res+= (s[i]-48) * p;
        }
        else if(s[i] >= 'a' && s[i]<='z')
        {
            res+= (s[i]-87) * p;
        }
        p<<=4;//p*=16
    }
    return res;
}
static int64_t hex_to_int64(const char* s)
{
    int64_t res = 0;
    int64_t p = 1;
    int32_t len = strlen(s);
    for(int32_t i=len-1;i>=0;i--)
    {
        if(s[i] >= '0' && s[i]<='9')
        {
            res+= (s[i]-48) * p;
        }
        else if(s[i] >= 'a' && s[i]<='z')
        {
            res+= (s[i]-87) * p;
        }
        
        p<<=4;
    }
    return res;
}
static const char* getOS()
{
  #ifdef __linux__
    return "Linux";
  #elif _WIN64
    return "Windows 64 bit";
  #elif _WIN32
    return "Windows 32 bit";
  #elif __FreeBSD__
    return "FreeBSD";
  #elif __APPLE__ || __MACH__
    return "Mac OSX";
  #elif __unix || __unix__
    return "Unix";
  #endif
  return "OS Unknown";
}
token resolveMacro(const char* name)
{
    token macro;
    macro.type = NUM_TOKEN;
    if(strcmp(name , "SEEK_CUR") == 0)
        macro.content = int64_to_string(SEEK_CUR);
    else if(strcmp(name , "SEEK_SET") == 0)
        macro.content = int64_to_string(SEEK_SET);
    else if(strcmp(name , "SEEK_END") == 0)
        macro.content = int64_to_string(SEEK_END);
    else if(strcmp(name , "pi") == 0)
    {
        macro.type = FLOAT_TOKEN;
        macro.content = clone_str("3.14159");
    }
    else if(strcmp(name , "e") == 0)
    {
        macro.type = FLOAT_TOKEN;
        macro.content = clone_str("2.718");
    }
    else if(strcmp(name , "clocks_per_sec") == 0)
    {
        macro.type = FLOAT_TOKEN;
        macro.content = int64_to_string(CLOCKS_PER_SEC);
    }
    else if(strcmp(name , "FLT_MIN") == 0)
    {
        macro.type=FLOAT_TOKEN;
        macro.content = double_to_string(-DBL_MAX);
    }
    else if(strcmp(name , "FLT_MAX") == 0)
    {
        macro.type=FLOAT_TOKEN;
        macro.content = double_to_string(DBL_MAX);
    }
    else if(strcmp(name , "INT_MIN") == 0)
        macro.content = int64_to_string(INT_MIN);
    else if(strcmp(name , "INT_MAX") == 0)
        macro.content = int64_to_string(INT_MAX);
    else if(strcmp(name , "INT64_MIN") == 0)
        macro.content = int64_to_string(LLONG_MIN);
    else if(strcmp(name , "INT64_MAX") == 0)        
        macro.content = int64_to_string(LLONG_MAX);
    else if(strcmp(name , "os") == 0)
    {
        macro.type = STRING_TOKEN;
        macro.content = clone_str(getOS());
    }
    else if(strcmp(name , "version") == 0)
    {
        macro.type = STRING_TOKEN;
        macro.content = clone_str("0.3.3");
    }
    else 
        macro.type = END_TOKEN;//to indicate macro not found
    return macro;

}
void str(lexer* ctx,token_vector* tokenlist)
{
    size_t j = ctx->k+1;
    bool match = false;
    bool escaped = false;
    size_t LN = ctx->line_num;
    dyn_str t;
    dyn_str_init(&t);
    const char* src = ctx->source_code;
    size_t src_length = ctx->srcLen;

    while(j < src_length)
    {
        if(src[j]=='"')
        {
            if(!escaped)
            {
                match = true;
                break;
            }
            escaped = false;
            dyn_str_push(&t, src[j]);
        }
        else if(src[j]=='\n')
        {
            dyn_str_push(&t, src[j]);
            ctx->line_num+=1;
        }
        else if(src[j]=='\\' && !escaped)
            escaped = true;
        else if(src[j] == '\\' && escaped)
        {
            dyn_str_push(&t, '\\');
            escaped = false;
        }
        else if(src[j] == 'a' && escaped)
        {
            dyn_str_push(&t, '\a');
            escaped = false;
        }
        else if(src[j] == 'r' && escaped)
        {
            dyn_str_push(&t, '\r');
            escaped = false;
        }
        else if(src[j] == 'n' && escaped)
        {
            dyn_str_push(&t, '\n');
            escaped = false;
        }
        else if(src[j] == 't' && escaped)
        {
            dyn_str_push(&t, '\t');
            escaped = false;
        }
        else if(src[j] == 'b' && escaped)
        {
            dyn_str_push(&t, '\b');
            escaped = false;
        }
        else if(src[j] == 'v' && escaped)
        {
            dyn_str_push(&t, '\v');
            escaped = false;
        }
        else if(src[j] == 'x' && escaped)
        {
            if(j + 2 >= src_length)
            {
                lexErr(ctx,"SyntaxError","Expected 2 hex digits after '\\x'");
                return;
            }
            if(!isalpha(src[j+1]) && !isdigit(src[j+1]))
            {
                lexErr(ctx,"SyntaxError","Expected 2 hex digits after '\\x'");
                return;
            }
            if(!isalpha(src[j+2]) && !isdigit(src[j+2]))
            {
                lexErr(ctx,"SyntaxError","Expected 2 hex digits after '\\x'");
                return;
            }
            char tmp[] = {src[j+1],src[j+2],0};
            unsigned char ch = tobyte(tmp);
            dyn_str_push(&t, ch);
            j += 2;
            escaped = false;
        }
        
        else
        {
            if(escaped) // unknown escape sequence
            {
                lexErr(ctx,"SyntaxError","Unknown escape character in string!");
                return;
            }
            dyn_str_push(&t, src[j]);
        }
        j+=1;
    }
    if(!match)
    {
        ctx->line_num = LN;
        lexErr(ctx,"SyntaxError","Error expected a '\"' at the end!");
        return;
    }
    if(escaped)
    {
        lexErr(ctx,"SyntaxError","String has non terminated escape sequence!");
        return;
    }
    token_vector_push(tokenlist,(make_token(STRING_TOKEN,t.arr,ctx->line_num)));
    ctx->k = j;
}
void macro(lexer* ctx,token_vector* tokenlist)
{
    const char* src = ctx->source_code;
    size_t src_length = ctx->srcLen;
    if(ctx->k==src_length-1)
    {
        lexErr(ctx,"SyntaxError","Invalid macro name");
        return;
    }
    size_t j = ctx->k+1;
    if(!isalpha(src[j]) && src[j]!='_')
    {
        ctx->k = src_length - 1;
        lexErr(ctx,"SyntaxError","Invalid macro name");
        return;
    }
    size_t begin = j;
    size_t end = begin;
    while(j<src_length)
    {
        if(!isalpha(src[j]) && !isdigit(src[j]) && src[j]!='_')
            break;
        end = j;
        j+=1;
    }
    char* t = make_substr(src,src_length,begin,end-begin+1);
    token tok;
    if(strcmp(t , "file") == 0)
    {
        tok.type = STRING_TOKEN;
        tok.content = clone_str(ctx->filename);
    }
    else if(strcmp(t , "lineno") == 0)
    {
        tok.type = NUM_TOKEN;
        tok.content = int64_to_string(ctx->line_num);
    }
    else
    {
        tok = resolveMacro(t);
        if(tok.type == END_TOKEN)
        {
            ctx->k = src_length - 1;
            snprintf(ctx->buffer,200,"Unknown macro @%s",t);
            free(t);
            lexErr(ctx,"NameError",ctx->buffer);
            return;
        }
    }
    free(t);
    token_vector_push(tokenlist,tok);
    ctx->k = j;
}
void comment(lexer* ctx,token_vector* tokenlist)
{
    bool multiline = false;
    bool foundEnd = false;
    const char* src = ctx->source_code;
    size_t src_length = ctx->srcLen;

    if(ctx->k+1 < src_length && src[ctx->k+1]=='-')
        multiline = true;
    size_t j=ctx->k+1;
    size_t orgLn = ctx->line_num;
    for(;j<src_length;j+=1)
    {
        if(!multiline && src[j]=='\n')
        {
            break;
        }
        else if(multiline && src[j] == '-')
        {
            if(j+1 < src_length && src[j+1]=='#')
            {
                foundEnd = true;
                j+=2;
                break;
            }
        }
        else if(src[j] == '\n')
            ctx->line_num++;
    }
    if(multiline && !foundEnd)
    {
        ctx->k = src_length - 1;
        ctx->line_num = orgLn;
        lexErr(ctx,"SyntaxError","Unable to find the end of multiline comment!");
        return;
    }
    ctx->k = j-1;
}
void numeric(lexer* ctx,token_vector* tokenlist)
{

    const char* src = ctx->source_code;
    size_t src_length = ctx->srcLen;
    char c = src[ctx->k];
    //hex literal
    if(c=='0' && ctx->k!=src_length-1 && src[ctx->k+1]=='x')
    {
        ctx->k+=1;
        size_t j = ctx->k+1;
        dyn_str b;
        dyn_str_init(&b);
        token i;
        while(j<src_length)
        {
            c = src[j];
            if(c>='0' && c<='9')
                dyn_str_push(&b,c);
            else if(c>='a' && c<='z')
                dyn_str_push(&b,c);
            else
                break;
            j+=1;

        }
        if(b.length == 1 || b.length == 2) //byte
        {
            if(b.length == 1)
                dyn_str_prepend_cstr(&b,"0");
            i.type = BYTE_TOKEN;
            i.ln = ctx->line_num;
            i.content = b.arr;
            token_vector_push(tokenlist,i);
        }
        else if(b.length >= 3 && b.length<=8)//int32
        {
            i.type = NUM_TOKEN;
            i.ln = ctx->line_num;
            i.content = int64_to_string(hex_to_int32(b.arr));
            free(b.arr);
            token_vector_push(tokenlist,i);
        }
        else if(b.length>8 &&  b.length <= 16)//int64
        {
            i.type = NUM_TOKEN;
            i.ln = ctx->line_num;
            i.content = int64_to_string(hex_to_int64(b.arr));
            free(b.arr);
            token_vector_push(tokenlist,i);
        }
        else
        {
            lexErr(ctx,"SyntaxError","Invalid Syntax");
            return;
        }
        ctx->k = j-1;
        return;
    }
    size_t j = ctx->k+1;
    dyn_str t;
    dyn_str_init(&t);
    dyn_str_push(&t,c);
    bool decimal = false;
    bool exp = false;
    bool expSign = false;
    while(j<src_length)
    {
        if(!isdigit(src[j]) && src[j]!='.' && src[j]!='e' && src[j]!='-' && src[j]!='+')
        {
            j = j-1;
            break;
        }
        if(src[j]=='.')
        {
            if(decimal || exp)
            {
                j = j-1;
                break;
            }
            decimal = true;
        }
        else if(src[j]=='e')
        {
            if(exp)
            {
                j = j-1;
                break;
            }
            exp = true;
        }
        else if(src[j]=='+' || src[j]=='-')
        {
            if(expSign || !exp)
            {
                j = j-1;
                break;
            }
            expSign = true;
        }
        dyn_str_push(&t, src[j]);
        j+=1;
    }

    if(t.arr[t.length-1] =='e') //t.arr has length of atleast 1 so we are safe
    {
        t.arr[t.length-1] = 0;
        j-=1;
    }
    token i;
    i.content = t.arr;
    if(!decimal && !exp)
        i.type = NUM_TOKEN;
    else
        i.type = FLOAT_TOKEN;
    i.ln = ctx->line_num;
    token_vector_push(tokenlist,i);
    ctx->k = j;
}
void id(lexer* ctx,token_vector* tokenlist)
{
    const char* src = ctx->source_code;
    size_t src_length = ctx->srcLen;
    size_t j = ctx->k+1;
    dyn_str t;
    dyn_str_init(&t);
    dyn_str_push(&t,src[ctx->k]);
    while(j<src_length)
    {
        if((j!=src_length-1 && src[j]==':' && src[j+1]==':'))
        {
            dyn_str_push(&t, ':');
            dyn_str_push(&t, ':');   
            j+=2;
        }
        else if(!isalpha(src[j]) && !isdigit(src[j]) && src[j]!='_')
        {
            j = j-1;
            break;
        }
        else
        {
            dyn_str_push(&t, src[j]);
            j+=1;
        }
    }

    token i;
    if(isKeyword(t.arr))
    {
        if(strcmp(t.arr,"if") == 0 && ctx->k!=0 && tokenlist->size!=0)
        {
            if(tokenlist->arr[tokenlist->size-1].type == KEYWORD_TOKEN && strcmp(tokenlist->arr[tokenlist->size-1].content,"else") == 0)
            {
                tokenlist->arr[tokenlist->size-1].content = clone_str("else if"); // memory leak
                ctx->k = j;
                return;
            }
        }
        i.type = KEYWORD_TOKEN;
        i.content = clone_str(t.arr);
        i.ln = ctx->line_num;
    }
    else if(strcmp(t.arr,"or") == 0 || strcmp(t.arr,"and") == 0 || strcmp(t.arr,"is") == 0)
    {
        i.content = t.arr;
        i.type = OP_TOKEN;
        i.ln = ctx->line_num;
    }
    else if(strcmp(t.arr,"true") == 0 || strcmp(t.arr,"false") == 0)
    {
        i.content = clone_str(t.arr);
        i.type = BOOL_TOKEN;
        i.ln = ctx->line_num;
    }
    else
    {
        i.type = ID_TOKEN;
        i.content = clone_str(t.arr);
        i.ln = ctx->line_num;
    }
    token_vector_push(tokenlist,i);
    ctx->k = j;
}
token_vector lexer_generateTokens(lexer* ctx,const zuko_src* src,bool printErr,size_t root_idx)
{
    token_vector tokenlist;
    token_vector_init(&tokenlist);
    if(root_idx >= src->files.size)
    {
        ctx->hadErr = true;
        ctx->errmsg = "root_idx out of range!";
        return tokenlist;
    }
    ctx->filename = src->files.arr[root_idx];
    ctx->source_code = src->sources.arr[root_idx];

    ctx->srcLen = strlen(ctx->source_code);
    ctx->printErr = printErr;
    ctx->hadErr = false;
    ctx->errmsg = NULL;
    ctx->line_num = 1;
    ctx->k = 0;
    char c;


    while(!ctx->hadErr && ctx->k<ctx->srcLen)
    {
        c = ctx->source_code[ctx->k];
        if(c=='"')
            str(ctx, &tokenlist);
        else if(c=='@')
        {
            macro(ctx,&tokenlist);
            continue;
        }
        else if(c=='#')
            comment(ctx, &tokenlist);
        else if(isdigit(c))
            numeric(ctx,&tokenlist);
        else if(c=='>' || c=='<')
        {
            if(ctx->k+1 < ctx->srcLen && ctx->source_code[ctx->k+1]=='=')
            {
                if(c == '<')
                    token_vector_push(&tokenlist,make_token(OP_TOKEN,"<=",ctx->line_num));
                else
                    token_vector_push(&tokenlist,make_token(OP_TOKEN,">=",ctx->line_num));
                ctx->k++;
            }
            else if(c=='<' && ctx->k+1<ctx->srcLen &&  ctx->source_code[ctx->k+1]=='<')
            {
                token_vector_push(&tokenlist,make_token(OP_TOKEN,"<<",ctx->line_num));
                ctx->k+=1;
            }
            else if(c=='>' && ctx->k+1 < ctx->srcLen && ctx->source_code[ctx->k+1]=='>')
            {
                token_vector_push(&tokenlist,make_token(OP_TOKEN,">>",ctx->line_num));
                ctx->k+=1;
            }
            else
                token_vector_push(&tokenlist,make_token(OP_TOKEN,((c == '>' ? ">" : "<")),ctx->line_num));
        }
        else if(c=='.')
            token_vector_push(&tokenlist,make_token(OP_TOKEN,("."),ctx->line_num));        
        else if(c=='+' || c=='-')
        {
            if(ctx->k!=ctx->srcLen-1 && ctx->source_code[ctx->k+1]=='=')
            {
                if(c == '+')
                    token_vector_push(&tokenlist,make_token(OP_TOKEN,"+=",ctx->line_num));
                else
                    token_vector_push(&tokenlist,make_token(OP_TOKEN,"-=",ctx->line_num));
                ctx->k+=2;
                continue;
            }
            token_vector_push(&tokenlist,make_token(OP_TOKEN,((c == '+') ? "+" : "-"),ctx->line_num));
        }
        else if(c=='/')
        {
            if(ctx->k!=ctx->srcLen-1 && ctx->source_code[ctx->k+1] == '=')
            {
                token_vector_push(&tokenlist,make_token(OP_TOKEN,"/=",ctx->line_num));
                ctx->k+=2;
                continue;
            }
            token_vector_push(&tokenlist,make_token(OP_TOKEN,"/",ctx->line_num));
        }
        else if(c=='*')
        {
            if(ctx->k!=ctx->srcLen-1 && ctx->source_code[ctx->k+1] == '=')
            {
                token_vector_push(&tokenlist,make_token(OP_TOKEN,"*=",ctx->line_num));
                ctx->k+=2;
                continue;
            }
            char* char_to_str(char ch);
            token_vector_push(&tokenlist,make_token(OP_TOKEN,"*",ctx->line_num));
        }
        else if(c=='%')
        {
            if(ctx->k!=ctx->srcLen-1 && ctx->source_code[ctx->k+1] == '=')
            {
                token_vector_push(&tokenlist,make_token(OP_TOKEN,"%=",ctx->line_num));
                ctx->k+=2;
                continue;
            }
            token_vector_push(&tokenlist,make_token(OP_TOKEN,"%",ctx->line_num));
        }
        else if(c=='^')
        {
            if(ctx->k!=ctx->srcLen-1 && ctx->source_code[ctx->k+1] == '=')
            {
                token_vector_push(&tokenlist,make_token(OP_TOKEN,"^=",ctx->line_num));
                ctx->k+=2;
                continue;
            }
            token_vector_push(&tokenlist,make_token(OP_TOKEN,"^",ctx->line_num));
        }
        else if(c=='&')
        {
            if(ctx->k!=ctx->srcLen-1 && ctx->source_code[ctx->k+1] == '=')
            {
                token_vector_push(&tokenlist,make_token(OP_TOKEN,"&=",ctx->line_num));
                ctx->k+=2;
                continue;
            }
            token_vector_push(&tokenlist,make_token(OP_TOKEN,"&",ctx->line_num));
        }
        else if(c=='|')
        {
            if(ctx->k!=ctx->srcLen-1 && ctx->source_code[ctx->k+1] == '=')
            {
                token_vector_push(&tokenlist,make_token(OP_TOKEN,"|=",ctx->line_num));
                ctx->k+=2;
                continue;
            }
            char* char_to_str(char ch);
            token_vector_push(&tokenlist,make_token(OP_TOKEN,"|",ctx->line_num));
        }
        else if(c=='~')
        {
            if(ctx->k!=ctx->srcLen-1 && ctx->source_code[ctx->k+1] == '=')
            {
                token_vector_push(&tokenlist,make_token(OP_TOKEN,"~=",ctx->line_num));
                ctx->k+=2;
                continue;
            }
            token_vector_push(&tokenlist,make_token(OP_TOKEN,"~",ctx->line_num));
        }
        else if(c=='!')
        {
            if(ctx->k!=ctx->srcLen-1 && ctx->source_code[ctx->k+1] == '=')
            {
                ctx->k+=1;
                token_vector_push(&tokenlist,make_token(OP_TOKEN,"!=",ctx->line_num));
            }
            else
                token_vector_push(&tokenlist,make_token(OP_TOKEN,"!",ctx->line_num));
        }
        else if(c=='=')
        {
            if(ctx->k!=ctx->srcLen-1 && ctx->source_code[ctx->k+1]=='=')
            {
                token_vector_push(&tokenlist,make_token(OP_TOKEN,("=="),ctx->line_num));
                ctx->k+=1;
            }
            else
                token_vector_push(&tokenlist,make_token(OP_TOKEN,("="),ctx->line_num));  
        }
        else if(c=='(')
            token_vector_push(&tokenlist,make_token(LParen_TOKEN,"(",ctx->line_num));
        else if(c==')')
            token_vector_push(&tokenlist,make_token(RParen_TOKEN,")",ctx->line_num));
        else if(c=='[')
            token_vector_push(&tokenlist,make_token(BEGIN_LIST_TOKEN,"[",ctx->line_num));
        else if(c==']')
            token_vector_push(&tokenlist,make_token(END_LIST_TOKEN,"]",ctx->line_num));
        else if(c=='{')
            token_vector_push(&tokenlist,make_token(L_CURLY_BRACKET_TOKEN,"{",ctx->line_num));
        else if(c=='}')
            token_vector_push(&tokenlist,make_token(R_CURLY_BRACKET_TOKEN,"}",ctx->line_num));
        else if(isalpha(c) || c=='_')
            id(ctx,&tokenlist);
        else if(c==',')
            token_vector_push(&tokenlist,make_token(COMMA_TOKEN,",",ctx->line_num));
        else if(c==':')
            token_vector_push(&tokenlist,make_token(COLON_TOKEN,":",ctx->line_num));
        else if(c=='\n' )
        {
            token_vector_push(&tokenlist,make_token(NEWLINE_TOKEN,"\n",ctx->line_num));
            ctx->line_num++;
        }
        else if(c=='\t' || c==' ' || c=='\r')
           ; //do nothing
        else
        {
            //error;
            snprintf(ctx->buffer,200, "Illegal character %c (ascii %d)",c,c);
            lexErr(ctx,"SyntaxError",ctx->buffer);
            return tokenlist;
        }
        ctx->k+=1;
    }
    if(ctx->hadErr)
      return tokenlist;
    return tokenlist;
}