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
#include "convo.h"
#include "repl.h"
#define ZUKO_VER_STRING "0.3.3"
void REPL();

const char* keywords[] = {
    "var",
    "if",
    "else",
    "while",
    "dowhile",
    "import",
    "return",
    "break",
    "continue",
    "function",
    "nil",
    "for",
    "to",
    "dto",
    "step",
    "foreach",
    "namespace",
    "class",
    "extends",
    "try",
    "catch",
    "throw",
    "yield",
    "as",
    "gc"
};

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
    fprintf(stderr,"\n%s\n",msg);
    if(REPL_MODE)
      repl();
}
bool matches(const char* a,const char* b,size_t b_length)
{
    return strncmp(a,b,b_length) == 0 && strlen(a) == b_length;
}
token resolve_macro(lexer_ctx* ctx,const char* name,size_t length)
{
    size_t lineno = ctx->line_num;
    if(matches(name , "SEEK_CUR",length))
        return make_token(NUM_TOKEN,int64_to_str(SEEK_CUR),lineno);
    else if(matches(name , "SEEK_SET",length))
        return make_token(NUM_TOKEN,int64_to_str(SEEK_SET),lineno);
    else if(matches(name , "SEEK_END",length))
        return make_token(NUM_TOKEN,int64_to_str(SEEK_END),lineno);
    else if(matches(name , "pi",length))
        return make_token(FLOAT_TOKEN,strdup("3.14159"),lineno);
    else if(matches(name , "e",length))
        return make_token(FLOAT_TOKEN,strdup("2.718"),lineno);
    else if(matches(name , "clocks_per_sec",length))
        return make_token(NUM_TOKEN,int64_to_str(CLOCKS_PER_SEC),lineno);
    else if(matches(name , "INT_MIN",length))
        return make_token(NUM_TOKEN,int64_to_str(INT_MIN),lineno);
    else if(matches(name , "INT_MAX",length))
        return make_token(NUM_TOKEN,int64_to_str(INT_MAX),lineno);
    else if(matches(name , "INT64_MIN",length))
        return make_token(NUM_TOKEN,int64_to_str(LLONG_MIN),lineno);
    else if(matches(name , "INT64_MAX",length) )        
        return make_token(NUM_TOKEN,int64_to_str(LLONG_MAX),lineno);
    else if(matches(name , "os",length))
        return make_token(STRING_TOKEN,strdup(get_os_name()),lineno);
    else if(matches(name , "version",length))
        return make_token(STRING_TOKEN,strdup(ZUKO_VER_STRING),lineno);
    else if(matches(name,"filename",length))
        return make_token(STRING_TOKEN,strdup(ctx->filename),lineno);
    else if(matches(name,"lineno",length))
        return make_token(STRING_TOKEN,int64_to_str(ctx->line_num),lineno);
    else
        return make_token(END_TOKEN,"",0); //to indicate macro not found
}
void str(lexer_ctx* ctx,token_vector* tokenlist)
{
    size_t j = ctx->k + 1;
    bool match = false;
    bool escaped = false;
    size_t LN = ctx->line_num;
    dyn_str t;
    dyn_str_init(&t);
    const char* src = ctx->source_code;
    size_t src_length = ctx->srcLen;

    while(j < src_length)
    {
        char ch = src[j]; // current character
        if(ch == '"')
        {
            if(!escaped)
            {
                match = true;
                break;
            }
            escaped = false;
            dyn_str_push(&t, ch);
        }
        else if(ch == '\n')
        {
            dyn_str_push(&t, ch);
            ctx->line_num+=1;
        }
        else if(ch == '\\' && !escaped)
            escaped = true;
        else if(ch == '\\' && escaped)
        {
            dyn_str_push(&t, '\\');
            escaped = false;
        }
        else if(ch == 'a' && escaped)
        {
            dyn_str_push(&t, '\a');
            escaped = false;
        }
        else if(ch == 'r' && escaped)
        {
            dyn_str_push(&t, '\r');
            escaped = false;
        }
        else if(ch == 'n' && escaped)
        {
            dyn_str_push(&t, '\n');
            escaped = false;
        }
        else if(ch == 't' && escaped)
        {
            dyn_str_push(&t, '\t');
            escaped = false;
        }
        else if(ch == 'b' && escaped)
        {
            dyn_str_push(&t, '\b');
            escaped = false;
        }
        else if(ch == 'v' && escaped)
        {
            dyn_str_push(&t, '\v');
            escaped = false;
        }
        else if(src[j] == 'x' && escaped)
        {
            bool hasMoreChars = (j+2 < src_length);
            bool invalidFirstChar = hasMoreChars && (!isalpha(src[j+1]) && !isdigit(src[j+1]));
            bool invalidSecondChar = hasMoreChars && (!isalpha(src[j+2]) && !isdigit(src[j+2]));
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
    if(ctx->k + 1 == src_length) //last character
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
    token tok = resolve_macro(ctx, src + begin, macro_name_length);
    if(tok.type == END_TOKEN)
    {
        ctx->k = src_length - 1;
        snprintf(ctx->buffer, 200, "Unknown macro");
        lexErr(ctx, "NameError", ctx->buffer);
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
            break;
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
    if(c == '0' && ctx->k != src_length-1 && src[ctx->k+1] == 'x')
    {
        ctx->k += 1;
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
            i.content = int64_to_str(hex_to_int32(b.arr));
            free(b.arr);
            token_vector_push(tokenlist,i);
        }
        else if(b.length>8 &&  b.length <= 16)//int64
        {
            i.type = NUM_TOKEN;
            i.ln = ctx->line_num;
            i.content = int64_to_str(hex_to_int64(b.arr));
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
    size_t j = ctx->k + 1;
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
            --j;
            break;
        }
        if(src[j]=='.')
        {
            if(decimal || exp)
            {
                --j;
                break;
            }
            decimal = true;
        }
        else if(src[j]=='e')
        {
            if(exp)
            {
                --j;
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
            --j;
            break;
        }
        else
        {
            dyn_str_push(&t, src[j]);
            ++j;
        }
    }
    //printf("id: %s\n",t.arr);
    token i;
    if(isKeyword(t.arr))
    {
        if(strcmp(t.arr,"if") == 0 && ctx->k!=0 && tokenlist->size!=0)
        {
            if(tokenlist->arr[tokenlist->size-1].type == KEYWORD_TOKEN && strcmp(tokenlist->arr[tokenlist->size-1].content,"else") == 0)
            {
                free(t.arr);
                free(tokenlist->arr[tokenlist->size-1].content);
                tokenlist->arr[tokenlist->size-1].content = strdup("else if");
                ctx->k = j;
                return;
            }
        }
        i.type = KEYWORD_TOKEN;
        i.content = t.arr;
        i.ln = ctx->line_num;
    }
    else if(strcmp(t.arr,"or") == 0 )
    {
        i.content = "or";
        i.type = OP_TOKEN;
        i.ln = ctx->line_num;
        free(t.arr);
    }
    else if(strcmp(t.arr,"and") == 0)
    {
        i.content = "and";
        i.type = OP_TOKEN;
        i.ln = ctx->line_num;
        free(t.arr);
    }
    else if(strcmp(t.arr,"is") == 0)
    {
        i.content = "is";
        i.type = OP_TOKEN;
        i.ln = ctx->line_num;
        free(t.arr);
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

token_vector tokenize(lexer_ctx* ctx,const zuko_src* src,bool printErr,size_t root_idx,size_t startidx)
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
    if(startidx == 0)
        ctx->line_num = 1;
    else
    {
        ctx->line_num = 1;
        for(size_t i = 0; i<startidx; i++)
        {
            if(ctx->source_code[i] == '\n')
                ctx->line_num++;
        }
    }
    ctx->k = startidx;
    char c;


    while(!ctx->hadErr && ctx->k < ctx->srcLen)
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
