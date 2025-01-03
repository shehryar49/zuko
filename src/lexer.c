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
#include "misc.h"
#define ZUKO_VER_STRING "0.3.3"
extern bool REPL_MODE;
void REPL();

const char* keywords[] = {"var","if","else","while","dowhile","import","return","break","continue","fn","nil","for","to","dto","step","foreach","namespace","class","private","public","extends","try","catch","throw","yield","as","gc"};
bool isKeyword(const char* s)
{
  for(size_t k=0;k<sizeof(keywords)/sizeof(char*);k+=1)
  {
    if(strcmp(s,keywords[k]) == 0)
      return true;
  }
  return false;
}

char* char_to_str(char ch)
{
    char tmp[2] = {ch,0};
    return strdup(tmp);
}
char* make_substr(const char* s,size_t strlen,size_t idx,size_t len)
{
    char* d = (char*)malloc(sizeof(char)*(strlen+1));
    memcpy(d,s+idx,len);
    d[len] = 0;
    return d;
}
void lexErr(lexer_ctx* ctx,const char* type,const char* msg)
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
token resolveMacro(lexer_ctx* ctx,const char* name,size_t length)
{
    size_t lineno = ctx->line_num;
    if(strncmp(name , "SEEK_CUR",length) == 0)
        return make_token(NUM_TOKEN,int64_to_string(SEEK_CUR),lineno);
    else if(strncmp(name , "SEEK_SET",length) == 0)
        return make_token(NUM_TOKEN,int64_to_string(SEEK_SET),lineno);
    else if(strncmp(name , "SEEK_END",length) == 0)
        return make_token(NUM_TOKEN,int64_to_string(SEEK_END),lineno);
    else if(strncmp(name , "pi",length) == 0)
        return make_token(FLOAT_TOKEN,strdup("3.14159"),lineno);
    else if(strncmp(name , "e",length) == 0)
        return make_token(FLOAT_TOKEN,strdup("2.718"),lineno);
    else if(strncmp(name , "clocks_per_sec",length) == 0)
        return make_token(NUM_TOKEN,int64_to_string(CLOCKS_PER_SEC),lineno);
    else if(strncmp(name , "INT_MIN",length) == 0)
        return make_token(NUM_TOKEN,int64_to_string(INT_MIN),lineno);
    else if(strncmp(name , "INT_MAX",length) == 0)
        return make_token(NUM_TOKEN,int64_to_string(INT_MAX),lineno);
    else if(strncmp(name , "INT64_MIN",length) == 0)
        return make_token(NUM_TOKEN,int64_to_string(LLONG_MIN),lineno);
    else if(strncmp(name , "INT64_MAX",length) == 0)        
        return make_token(NUM_TOKEN,int64_to_string(LLONG_MAX),lineno);
    else if(strncmp(name , "os",length) == 0)
        return make_token(STRING_TOKEN,strdup(getOS()),lineno);
    else if(strncmp(name , "version",length) == 0)
        return make_token(STRING_TOKEN,strdup(ZUKO_VER_STRING),lineno);
    else if(strncmp(name,"filename",length) == 0)
        return make_token(STRING_TOKEN,strdup(ctx->filename),lineno);
    else if(strncmp(name,"lineno",length) == 0)
        return make_token(STRING_TOKEN,int64_to_string(ctx->line_num),lineno);
    else
        return make_token(END_TOKEN,"",0); //to indicate macro not found
}
void str(lexer_ctx* ctx,token_vector* tokenlist)
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
            bool noMoreChars = (j+2 >= src_length);
            bool invalidFirstChar = !noMoreChars && (!isalpha(src[j+1]) && !isdigit(src[j+1]));
            bool invalidSecondChar = !noMoreChars && (!isalpha(src[j+2]) && !isdigit(src[j+2]));
            if(invalidFirstChar || invalidSecondChar)
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
void macro(lexer_ctx* ctx,token_vector* tokenlist)
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
    size_t macro_name_length = end - begin + 1;
    token tok = resolveMacro(ctx,src+begin,macro_name_length);
    if(tok.type == END_TOKEN)
    {
        ctx->k = src_length - 1;
        snprintf(ctx->buffer,200,"Unknown macro");
        lexErr(ctx,"NameError",ctx->buffer);
        return;
    }
    token_vector_push(tokenlist,tok);
    ctx->k = j;
}
void comment(lexer_ctx* ctx,token_vector* tokenlist)
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
void numeric(lexer_ctx* ctx,token_vector* tokenlist)
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
        while(j < src_length)
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
                dyn_str_prepend(&b,"0");
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
void id(lexer_ctx* ctx,token_vector* tokenlist)
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
        i.content = t.arr;
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
        i.content = t.arr;
        i.type = BOOL_TOKEN;
        i.ln = ctx->line_num;
    }
    else
    {
        i.type = ID_TOKEN;
        i.content = t.arr;
        i.ln = ctx->line_num;
    }
    token_vector_push(tokenlist,i);
    ctx->k = j;
}
token_vector tokenize(lexer_ctx* ctx,const zuko_src* src,bool printErr,size_t root_idx)
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
