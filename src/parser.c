#include "parser.h"
#include "ast.h"
#include "dyn-str.h"
#include "lexer.h"
#include <string.h>
#include "nodeptr_vec.h"
#include "ptr-vector.h"
#include "refgraph.h"
#include "str-vec.h"
#include "token-vector.h"
#include "token.h"
#include "zuko-src.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
extern bool REPL_MODE;
void REPL();


Node* new_node(NodeType type,const char* val)
{
  Node* p = malloc(sizeof(Node));
  if(!p)
  {
    printf("Parser: Error allocating memory for AST!\n");
    exit(1);
  }
  p->val = val;
  p->type = type;
  nodeptr_vector_init(&p->childs);
  return p;
}

void strip_newlines(token* tokens,int* begin,int* end)
{
    while(*end >= *begin && tokens[*begin].type==NEWLINE_TOKEN )
        *begin+=1; 
    while(*end >= *begin && tokens[*end].type==NEWLINE_TOKEN )
        *end-=1;
}
char* merge_str(const char* str1,const char* str2)
{
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);

    char* result = (char*)malloc(sizeof(char)*(len1+len2+1));

    memcpy(result,str1,len1);
    memcpy(result+len1,str2,len2);
    result[len1+len2] = 0;
    return result;
}
//Function to find match for tokens like '(' '[' '{'
int match_token(int k,int endidx,TokenType end,token* tokens)
{
    TokenType start = tokens[k].type;
    int ignore = 0;
    while(k<=endidx)
    {
        if(tokens[k].type  == start)
            ignore+=1;
        else if(tokens[k].type== end)
        {
            ignore-=1;
            if(ignore==0)
                return k;
        }
        k+=1;
    }
    return -1;
}
int match_token_right(int k,int begin,TokenType end,token* tokens)
{
    TokenType start = tokens[k].type;
    
    int ignore = 0;
    while(k>=begin)
    {
        if(tokens[k].type == start)
            ignore+=1;
        else if(tokens[k].type == end)
        {
            ignore-=1;
            if(ignore==0)
                return k;
        }
        k-=1;

    }
    return -1;
}

int find_token(token t,int start,int end,token* tokens)
{
   while(start<=end)
   {
     if(tokens[start].type==t.type && strcmp(tokens[start].content,t.content) == 0)
       return start;
     start+=1;
   }
   return -1;
}

const char* NodeTypeNames[] =
{
  "declare",
  "import",
  "importas",
  "assign",
  "memb",
  "WHILE",
  "DOWHILE",
  "FOR",
  "DFOR",
  "decl",//used with FOR loop to tell if loop control variable should be
  //declared or not
  //i.e for(var i=0 to 10 step 1)
  "nodecl",
  "FOREACH",
  "NAMESPACE",
  "IF",
  "IFELSE",
  "IFELIF",
  "IFELIFELSE",
  "THROW",
  "TRYCATCH",
  "FUNC",
  "CORO",
  "CLASS",
  "EXTCLASS",
  "RETURN",
  "EOP", //end of "program", also marks the end of tree
  "YIELD",
  "BREAK",
  "CONTINUE",
  "call",
  "args",
  "line",
  "gc",
  "file",
  "end",
  "add",
  "sub",
  "mul",
  "div",
  "mod",
  "XOR",
  "lshift",
  "rshift",
  "bitwiseand",
  "bitwiseor",
  "AND",
  "OR",
  "IS",
  "lt",
  "gt",
  "lte",
  "gte",
  "equal",
  "noteq",
  "neg",
  "complement",
  "NOT",
  "index",
  "dict",
  "list",
  "NUM",
  "FLOAT",
  "NIL",
  "STR",
  "BOOL",
  "BYTE",
  "ID",
  "conditions"
};

void delete_ast(Node* ast)
{
  if(!ast)
    return;
  for(size_t k = 0;k<ast->childs.size;k+=1)
  {
     delete_ast(ast->childs.arr[k]);
  }
  nodeptr_vector_destroy(&ast->childs);
  if(ast->type == line_node)
    free((void*)ast->val);
  free(ast);
}

//For debugging purposes
/*const char* StrTokenTypes[] =
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
  "EOP"
};
*/
// Function to print AST in tablular form
void print_ast(Node* n,int spaces)
{
   if(!n)
    return;
   printf("|%s %s\n",NodeTypeNames[(int)n->type],n->val);
   spaces+=2;
   for(size_t k=0;k<(n->childs.size);k+=1)
   {
        for(int j=0;j<spaces;j++)
           fputc(' ',stdout);
        print_ast(n->childs.arr[k],spaces);
   }
}
int find_token_consecutive(token t,int start,int end,token* tokens)
{
    for(int k=start;k<=end;k+=1)
    {
        if(tokens[k].type==t.type && strcmp(tokens[k].content,t.content) == 0)
            return k;
        else if(tokens[k].type== NEWLINE_TOKEN)
            ;
        else
            return -1;
    }
    return -1;
}
bool atGlobalLevel(Parser* ctx)
{
return (!ctx->infunc && !ctx->inclass && !ctx->inloop
    && !ctx->inif && !ctx->inelif && !ctx->inelse && !ctx->intry && !ctx->incatch);
}
bool isValidCtxForFunc(Parser* ctx)
{
    return (!ctx->infunc  && !ctx->inloop
    && !ctx->inif && !ctx->inelif && !ctx->inelse && !ctx->intry && !ctx->incatch);
}
void parser_init(Parser* ctx)
{
    str_vector_init(&ctx->prefixes);
    str_vector_push(&ctx->prefixes,"");
    token_vector_init(&ctx->known_constants);
}
void parser_set_source(Parser* ctx,zuko_src* p,size_t root_idx)
{
    if(root_idx >= p->files.size)
    {
        fprintf(stderr,"Parser.init() failed. Invalid root_idx!");
        return;
    }
    ctx->num_of_constants = &p->num_of_constants;
    ctx->refGraph = &p->ref_graph;
    ctx->files = &p->files;
    ctx->sources = &p->sources;
    ctx->filename = p->files.arr[root_idx];
    ctx->currSym = ".main";
    ctx->inclass = false;
    ctx->infunc = false;
    ctx->inloop = false;
    ctx->inif = false;
    ctx->inelif = false;
    ctx->inelse = false;
    ctx->intry = false;
    ctx->incatch = false;
    ctx->foundYield = false;
    str_vector v;
    str_vector_init(&v);
    refgraph_emplace(ctx->refGraph,clone_str(".main"),v);
}
void parseError(Parser* ctx,const char* type,const char* msg)
{
    fprintf(stderr,"\nFile %s\n",ctx->filename);
    fprintf(stderr,"%s at line %zu\n",type,ctx->line_num);

    int idx = str_vector_search(ctx->files,ctx->filename);
    const char* source_code = ctx->sources->arr[idx];
    size_t l = 1;
    size_t k = 0;
    while(source_code[k]!=0 && l<=ctx->line_num)
    {
        if(source_code[k]=='\n')
            l+=1;
        else if(l==ctx->line_num)
        {
            fputc(source_code[k],stderr);
        }
        k+=1;
    }
    puts("");
    fprintf(stderr,"%s\n",msg);
    //if(REPL_MODE)
    //    REPL();
    exit(1);
}

bool addSymRef(Parser* ctx,const char* name)
{
    /* adds edge from currSym to name, indicating curr symbol uses name*/
    if(name == ctx->currSym) // no self loops
        return false;
    if(!refgraph_getref(ctx->refGraph,name))
        return false;
    str_vector* neighbours = refgraph_getref(ctx->refGraph,ctx->currSym);
    if(str_vector_search(neighbours,name) == -1)
        str_vector_push(neighbours,clone_str(name));
    return true;
}
int find_op(const char* prec[5][7],int i,const char* op)
{
    int j = 0;
    while(prec[i][j] != NULL)
    {
        if(strcmp(prec[i][j],op) == 0)
            return j;
        j+=1;
    }
    return -1;
}
static bool isnum(const char* s)
{
    while(s[0]=='0' && s[1]!=0)
      s++;
    char tmp[50];
    int x = atoi(s);
    snprintf(tmp,50,"%d",x);
	return strcmp(tmp,s) == 0;
}

Node* parseExpr(Parser* ctx,token* tokens,int begin,int end)
{   
    if(begin > end)
        parseError(ctx,"SyntaxError","Invalid Syntax");
    size_t tokens_size = end - begin + 1;

    Node* ast = NULL;
    if(tokens_size==1)
    {

        if(tokens[begin].type== KEYWORD_TOKEN && strcmp(tokens[begin].content,"nil")==0 )
        {
            ast = new_node(NIL,tokens[begin].content);
            return ast;
        }
        else if(tokens[begin].type== BOOL_TOKEN )
        {
            ast = new_node(BOOL_NODE,tokens[begin].content);
            return ast;
        }
        else if(tokens[begin].type == ID_TOKEN)
        {
            bool done = false;
            for(size_t i = 1;i <= ctx->prefixes.size; i++)
            {
                size_t idx = ctx->prefixes.size - i;
                ctx->aux = merge_str(ctx->prefixes.arr[idx],tokens[begin].content);
                if((done = addSymRef(ctx,ctx->aux)))
                    break;
                free((void*)ctx->aux);
            }
            ast = new_node(ID_NODE,tokens[begin].content);
            return ast;
        }
        else if(tokens[begin].type == NUM_TOKEN && isnum(tokens[begin].content))
        {
            ast = new_node(NUM,tokens[begin].content);//int32
            return ast;
        }
        else if(tokens[begin].type == BYTE_TOKEN)
        {
            ast = new_node(BYTE_NODE,tokens[begin].content);
            return ast;
        }
        else if(tokens[begin].type == STRING_TOKEN)
        {
            ast = new_node(STR_NODE,tokens[begin].content);
            return ast;
        }
        else if(tokens[begin].type== FLOAT_TOKEN || tokens[begin].type== NUM_TOKEN)
        {
            if(find_token(tokens[begin],0,(int)ctx->known_constants.size-1,ctx->known_constants.arr)==-1)
            {
                token_vector_push(&ctx->known_constants,tokens[begin]);
                *ctx->num_of_constants = *ctx->num_of_constants + 1;
            }
            if(tokens[begin].type == FLOAT_TOKEN)
                ast = new_node(FLOAT,tokens[begin].content);
            else if(tokens[begin].type == NUM_TOKEN)//int64
                ast = new_node(NUM,tokens[begin].content);
            return ast;
        }
        parseError(ctx,"SyntaxError","Invalid Syntax");
    }


    /////////
    //Following precdence is in reverse order
    //static vector<vector<string>> prec = {{"and","or","is"},{"<",">","<=",">=","==","!="},{"<<",">>","&","|","^"},{"+","-"},{"/","*","%"}};
    static const char* prec[5][7] = {{"and","or","is",NULL},{"<",">","<=",">=","==","!=",NULL},{"<<",">>","&","|","^",NULL},{"+","-",NULL},{"/","*","%",NULL}};
    static NodeType op_node_types[5][7] = {
            {AND,OR,IS_node},
            {lt,gt,lte,gte,equal,noteq},
            {lshift,rshift,bitwiseand,bitwiseor,XOR_node},
            {add,sub},
            {div_node,mul,mod}
    };
    static int l = 5;
    int k = tokens_size-1;
    
    for(int i=0;i<l;++i)
    {
        //vector<string>& op_set = prec[i];
        k = end;;
        int m = 0;
        size_t size_max = -1;
        while(k >=begin)
        {

        if(tokens[k].type== RParen_TOKEN)
        {
            int i = match_token_right(k,begin,LParen_TOKEN,tokens);
            if(i==-1)
                parseError(ctx,"SyntaxError","Invalid Syntax");
            k = i;
        }
        else if(tokens[k].type== END_LIST_TOKEN)
        {
            int i = match_token_right(k,begin,BEGIN_LIST_TOKEN,tokens);
            if(i==-1)
                parseError(ctx,"SyntaxError","Invalid Syntax");
            k=i;
        }
        else if(tokens[k].type== R_CURLY_BRACKET_TOKEN)
        {
            int i = match_token_right(k,begin,L_CURLY_BRACKET_TOKEN,tokens);
            if(i==-1)
                parseError(ctx,"SyntaxError","Invalid Syntax");
            k=i;
        }
        else if(tokens[k].type== OP_TOKEN && ((m = find_op(prec,i,tokens[k].content))!=-1) && k!=begin)
        {
            Node* ast = new_node(op_node_types[i][m],"");
            nodeptr_vector_push(&(ast->childs),parseExpr(ctx,tokens,begin,k-1));
            nodeptr_vector_push(&(ast->childs),parseExpr(ctx,tokens,k+1,end));
            return ast;
        }
        k-=1;
        }
    }
    //////

    /////////////////
    //Unary Operators
    ////////////////
    if(tokens[begin].type== OP_TOKEN)
    {

        if(strcmp(tokens[begin].content,"-") == 0)
        {
            Node* ast = new_node(neg,"");
            size_t sz = ctx->known_constants.size;
            Node* E = parseExpr(ctx,tokens,begin+1,end);
            if(ctx->known_constants.size==sz+1 && begin+1==end && (ctx->known_constants.arr[sz].type==FLOAT_TOKEN || ctx->known_constants.arr[sz].type==NUM_TOKEN))
            {
                ctx->known_constants.arr[sz].content = merge_str("-",ctx->known_constants.arr[sz].content);
                if(ctx->known_constants.arr[sz].type == FLOAT_TOKEN)
                {
                    ast->type = FLOAT;
                    ast->val = ctx->known_constants.arr[sz].content;
                }
                else if(ctx->known_constants.arr[sz].type == NUM_TOKEN)
                {
                    ast->type = NUM;
                    ast->val = ctx->known_constants.arr[sz].content;
                }
                return ast;
            }
            nodeptr_vector_push(&(ast->childs),E);
            return ast;
        }
        else if(strcmp(tokens[begin].content,"+") == 0)
        {
            return (parseExpr(ctx,tokens,begin+1,end));
        }
        else if(strcmp(tokens[begin].content,"~") == 0)
        {
            Node* ast = new_node(complement,"");
            nodeptr_vector_push(&(ast->childs),parseExpr(ctx,tokens,begin+1,end));
            return ast;
        }
        else if(strcmp(tokens[begin].content,"!") == 0)
        {
            Node* ast = new_node(NOT_node,"");
            nodeptr_vector_push(&(ast->childs),parseExpr(ctx,tokens,begin+1,end));
            return ast;
        }
    }
    if(tokens[end].type==END_LIST_TOKEN)// evaluate indexes or lists
    {
        int i = match_token_right(end,begin,BEGIN_LIST_TOKEN,tokens);
        if(i==-1)
            parseError(ctx,"SyntaxError","Invalid Syntax");
        if(i != begin)
        {
            Node* ast = new_node(index_node,"");
            nodeptr_vector_push(&(ast->childs),parseExpr(ctx,tokens,begin,i-1));
            nodeptr_vector_push(&(ast->childs),parseExpr(ctx,tokens,i+1,end-1));
            return ast;
        }
        else
        {

            ast = new_node(list,"");
            if(tokens_size==2)//empty list
                return ast;
            int L = end-1;//number of tokens excluding square brackets
            int start_elem = begin+1;
            int k = 0;
            for(k = begin+1;k<=L;k+=1)//process list elements
            {
                if(tokens[k].type== COMMA_TOKEN)
                {
                    int end_elem = k-1;
                    if(start_elem > end_elem)
                        parseError(ctx,"SyntaxError","Error expected an element before ',' ");
                    nodeptr_vector_push(&(ast->childs),parseExpr(ctx,tokens,start_elem,end_elem));
                    start_elem = k+1;
                }
                else if(tokens[k].type== BEGIN_LIST_TOKEN)
                {
                    int R = match_token(k,end,END_LIST_TOKEN,tokens);
                    if(R>(L) || R==-1)
                        parseError(ctx,"SyntaxError","Invalid Syntax");

                    k = R;
                }
                else if(tokens[k].type== L_CURLY_BRACKET_TOKEN)
                {
                    int R = match_token(k,end,R_CURLY_BRACKET_TOKEN,tokens);
                    if(R>L || R==-1)
                        parseError(ctx,"SyntaxError","Invalid Syntax");
                    //vector<token> subdict = {tokens.begin()+k,tokens.begin()+R+1};
                    //t.insert(t.end(),subdict.begin(),subdict.end());
                    k = R;
                }
                else if(tokens[k].type==ID_TOKEN && tokens[k+1].type==LParen_TOKEN)
                {
                    int R = match_token(k,end,RParen_TOKEN,tokens);
                    if(R>L || R==-1)
                        parseError(ctx,"SyntaxError","Invalid Syntax");
                    //vector<token> subfn = {tokens.begin()+k,tokens.begin()+R+1};
                    //t.insert(t.end(),subfn.begin(),subfn.end());
                    k = R;
                }

            }
            int elem_end = k - 1;
            if(start_elem > elem_end)
                parseError(ctx,"SyntaxError","Error expected an element after ',' ");
            nodeptr_vector_push(&(ast->childs),parseExpr(ctx,tokens,start_elem,elem_end));
            return ast;
        }
    }
    /////////////////
    k = end;
    while(k>=begin)
    {
        if(tokens[k].type==RParen_TOKEN)
        {
            int i = match_token_right(k,begin,LParen_TOKEN,tokens);
            if(i==-1)
                parseError(ctx,"SyntaxError","Invalid Syntax");
            k = i;
        }
        else if(tokens[k].type== END_LIST_TOKEN)
        {
            int i = match_token(k,end,BEGIN_LIST_TOKEN,tokens);
            if(i==-1)
                parseError(ctx,"SyntaxError","Invalid Syntax");
            k=i;
        }
        else if(tokens[k].type== R_CURLY_BRACKET_TOKEN)
        {
            int i = match_token_right(k,begin,L_CURLY_BRACKET_TOKEN,tokens);
            if(i==-1)
                parseError(ctx,"SyntaxError","Invalid Syntax");
            k = i;
        }
        else if(tokens[k].type== OP_TOKEN && (strcmp(tokens[k].content,".")==0))
        {
            Node* ast = new_node(memb,"");
            nodeptr_vector_push(&(ast->childs),parseExpr(ctx,tokens,begin,k-1));
            nodeptr_vector_push(&(ast->childs),parseExpr(ctx,tokens,k+1,end));
            return ast;
        }
        k-=1;
    }
    ////////////////
    ////////////////////
    if(tokens_size>=2)
    {
        if(tokens[begin].type== L_CURLY_BRACKET_TOKEN && tokens[end].type==R_CURLY_BRACKET_TOKEN)
        {
                ast = new_node(dict,"");
                if(tokens_size==2)
                    return ast;
                int key_begin = begin+1;
                int value_begin = 0;
                const char* tofill = "key";
                int L = end-1;
                for(k = begin+1;k<=L;k+=1)
                {
                    if(tokens[k].type== COMMA_TOKEN)
                    {
                        int value_end = k-1;
                        if(value_begin > value_end)
                            parseError(ctx,"SyntaxError","Invalid Syntax");
                        nodeptr_vector_push(&(ast->childs),parseExpr(ctx,tokens,value_begin,value_end));
                        key_begin = k+1;
                        tofill = "key";
                    }
                    else if(tokens[k].type== COLON_TOKEN)
                    {
                        int key_end = k-1;
                        if(key_begin > key_end)
                            parseError(ctx,"SyntaxError","Invalid Syntax");
                        nodeptr_vector_push(&(ast->childs),parseExpr(ctx,tokens,key_begin,key_end));

                        tofill = "value";
                        value_begin = k+1;
                    }
                    else if(tokens[k].type== BEGIN_LIST_TOKEN)
                    {
                        int R = match_token(k,L,END_LIST_TOKEN,tokens);
                        if(R == -1)
                            parseError(ctx,"SyntaxError","Invalid Syntax");
                        k = R;
                    }
                    else if(tokens[k].type== L_CURLY_BRACKET_TOKEN)
                    {
                        int R = match_token(k,L,R_CURLY_BRACKET_TOKEN,tokens);
                        if(R==-1)
                            parseError(ctx,"SyntaxError","Invalid Syntax");
                        k = R;
                    }
                    else if(tokens[k].type==ID_TOKEN && tokens[k+1].type==LParen_TOKEN)
                    {
                        int R = match_token(k,L,RParen_TOKEN,tokens);
                        if(R==-1)
                            parseError(ctx,"SyntaxError","Invalid Syntax");
                        k = R;
                    }
                    
                }
                int value_end = k-1;
                if(value_begin > value_end)
                    parseError(ctx,"SyntaxError","Invalid Syntax");

                nodeptr_vector_push(&(ast->childs),parseExpr(ctx,tokens,value_begin,value_end));
                return ast;
            
        }
    }
    ////////////////////
    ////////////////////
    if(tokens[begin].type== ID_TOKEN && tokens[begin+1].type== LParen_TOKEN)
    {
        int i = match_token(begin+1,end,RParen_TOKEN,tokens);
        if(i==-1)
            parseError(ctx,"SyntaxError","Invalid Syntax");
        if(i==end)
        {
            Node* ast = new_node(call,"");
            Node* args = new_node(args_node,"");

            Node* n = new_node(line_node,int64_to_string(tokens[begin].ln));
            nodeptr_vector_push(&(ast->childs),n);
            nodeptr_vector_push(&(ast->childs),new_node(ID_NODE,tokens[begin].content));
            bool done = false;
            for(size_t i = 1;i <= ctx->prefixes.size; i++)
            {
                size_t idx = ctx->prefixes.size - i;
                ctx->aux = merge_str(ctx->prefixes.arr[idx],tokens[begin].content);
                if((done = addSymRef(ctx,ctx->aux)))
                {
                    free((void*)ctx->aux);                    
                    break;
                }
                free((void*)ctx->aux);
            }
//            vector<token> Args = {tokens.begin()+begin+2,tokens.begin()+end};
            int args_begin = begin+2;
            int args_end = end-1;
            if(args_begin > args_end)
            {
                nodeptr_vector_push(&(ast->childs),args);
                return ast;
            }
            int arg_begin = args_begin;
            int k;
            for(k = args_begin;k<=args_end;k+=1)
            {
                if(tokens[k].type== COMMA_TOKEN)
                {
                    int arg_end = k-1;
                    if(arg_begin > arg_end)
                        parseError(ctx,"SyntaxError","Invalid Syntax");
                    nodeptr_vector_push(&(args->childs),parseExpr(ctx,tokens,arg_begin,arg_end));
                    arg_begin = k+1;
                }
                else if(tokens[k].type== BEGIN_LIST_TOKEN)
                {
                    int i = match_token(k,args_end,END_LIST_TOKEN,tokens);
                    if(i==-1)
                        parseError(ctx,"SyntaxError","Invalid Syntax");
                    //vector<token> LIST =  {Args.begin()+k,Args.begin()+i+1};
                    //T.insert(T.end(),LIST.begin(),LIST.end());
                    k = i;
                }
                else if(tokens[k].type== LParen_TOKEN)
                {
                    int i = match_token(k,args_end,RParen_TOKEN,tokens);
                    if(i==-1)
                        parseError(ctx,"SyntaxError","Invalid Syntax");
                    //vector<token> P =  {Args.begin()+k,Args.begin()+i+1};
                    //T.insert(T.end(),P.begin(),P.end());
                    k = i;
                }
                else if(tokens[k].type== L_CURLY_BRACKET_TOKEN)
                {
                    int i = match_token(k,args_end,R_CURLY_BRACKET_TOKEN,tokens);
                    if(i==-1)
                        parseError(ctx,"SyntaxError","Invalid Syntax");
                    //vector<token> P =  {Args.begin()+k,Args.begin()+i+1};
                    //T.insert(T.end(),P.begin(),P.end());
                    k = i;
                }
            }
            int arg_end = k - 1;
            if(arg_begin > arg_end)
                parseError(ctx,"SyntaxError","Invalid Syntax bitch");
            nodeptr_vector_push(&(args->childs),parseExpr(ctx,tokens,arg_begin,arg_end));
            nodeptr_vector_push(&(ast->childs),args);
            return ast;
        }//
    }
    ///////////////////
    //////////////
    if(tokens[begin].type== LParen_TOKEN)
    {
        int i = match_token(begin,end,RParen_TOKEN,tokens);
        if(i==-1 || tokens_size == 2)
            parseError(ctx,"SyntaxError","Invalid Syntax");
        if(i==(int)end)
        {
            return parseExpr(ctx,tokens,begin+1,end-1);
        }
    }
    bool multiline=false;
    const char* expr;
    for(int i=begin;i<=end;i++)
    {
        token e = tokens[i];
        if(e.type==NEWLINE_TOKEN)
        {
            multiline=true;
            break;
        }
        if(e.type==STRING_TOKEN)
            expr = merge_str(expr,merge_str(merge_str("\"" ,e.content), "\""));
        else
            expr = merge_str(expr,e.content);
    }
    if(!multiline)
    {
        char buffer[50];
        snprintf(buffer,50,"Invalid Syntax: %s",expr);
        parseError(ctx,"SyntaxError",buffer);
    }
    else
        parseError(ctx,"SyntaxError",expr);  
    return ast;
}
void add_child(Node* ast,Node* ptr)
{
    nodeptr_vector_push(&(ast->childs),ptr);
}
Node* parseStmt(Parser* ctx,token* tokens,int begin,int end)
{
    size_t tokens_size = end-begin+1;
    if(tokens_size==0)
        parseError(ctx,"SyntaxError","Invalid Syntax");

    if(tokens_size==1 && tokens[begin].type==KEYWORD_TOKEN && strcmp(tokens[begin].content,"gc") == 0)
        return new_node(gc,"");

    bool isPrivate = false;
    if(tokens[begin].type==KEYWORD_TOKEN && strcmp(tokens[begin].content,"private") == 0)
    {
        isPrivate = true;
        begin+=1;
        if(!ctx->inclass)
            parseError(ctx,"SyntaxError","Error use of keyword private outside class!");

        if(tokens[begin].type==KEYWORD_TOKEN && strcmp(tokens[begin].content,"var") == 0);
        else if(tokens[begin].type==KEYWORD_TOKEN && strcmp(tokens[begin].content,"function") == 0);
        else
            parseError(ctx,"SyntaxError","Invalid use of keyword private\n");
    }
    else if(tokens[begin].type==KEYWORD_TOKEN && strcmp(tokens[begin].content,"public") == 0)
    {
        isPrivate = false;
        //tokens.erase(tokens.begin());
        begin += 1;
        if(!ctx->inclass)
            parseError(ctx,"SyntaxError","Error use of keyword public outside class!");
        if(tokens[begin].type==KEYWORD_TOKEN && strcmp(tokens[begin].content,"var") == 0);
        else if(tokens[begin].type==KEYWORD_TOKEN && strcmp(tokens[begin].content,"function") == 0);
        else
            parseError(ctx,"SyntaxError","Invalid use of keyword public");
    }

    if(tokens[begin].type== KEYWORD_TOKEN  && strcmp(tokens[begin].content,"var") == 0)
    {
        //declare statement
        if(tokens_size < 4) // there must be minimum four tokens var x  (simplest case)        
            parseError(ctx,"SyntaxError","Invalid declare statement!");

        if(tokens[begin+1].type!= ID_TOKEN || (tokens[begin+2].type!= OP_TOKEN &&  strcmp(tokens[begin+2].content,"=")!=0))
            parseError(ctx,"SyntaxError","invalid declare statement!");
    
        Node* ast = new_node(declare,"");
        Node* n = new_node(line_node,int64_to_string(tokens[begin].ln));
        nodeptr_vector_push(&(ast->childs),n);
        //if(((string)tokens[1].content).find("::")!=string::npos)
        //    parseError(ctx,"SyntaxError","Invalid Syntax");
        //str_find("::",tokens[1].content);
        const char* fnprefix = ctx->prefixes.arr[ctx->prefixes.size-1];
        const char* tmp = tokens[begin+1].content;
        if(atGlobalLevel(ctx))
        {
            tmp = merge_str(fnprefix,tokens[begin+1].content);
    //        puts("adding to_free");
//            ptr_vector_push(&ctx->to_free,(void*)tmp);
  //          puts("done");
        }
        if(isPrivate)
        {
            tmp = merge_str("@",tokens[begin+1].content);
     //       puts("adding to _free2");
       //     ptr_vector_push(&ctx->to_free,(void*)tmp); 
        }
        ast->val = tmp;
        int expr_begin = begin+3;
        int expr_end = end;
        if(tokens[expr_begin].type==KEYWORD_TOKEN && strcmp(tokens[expr_begin].content , "yield") == 0)
        {
            ctx->foundYield = true;
            expr_begin++;
            nodeptr_vector_push(&(ast->childs),new_node(YIELD_node,""));
            if(expr_begin > expr_end)
                parseError(ctx,"SyntaxError","Error expected expression after return keyword");
            Node* n = new_node(line_node ,int64_to_string(tokens[begin].ln));
            Node* tmp = ast->childs.arr[ast->childs.size-1];
            nodeptr_vector_push(&(tmp->childs),n);
            add_child(ast->childs.arr[ast->childs.size-1],parseExpr(ctx,tokens,expr_begin,expr_end));
            return ast;
        }
        nodeptr_vector_push(&(ast->childs),parseExpr(ctx,tokens,expr_begin,expr_end));
        return ast;
    }

    if(tokens[begin].type== KEYWORD_TOKEN && (strcmp(tokens[begin].content,"break")==0 || strcmp(tokens[begin].content,"continue") == 0) && begin==end)
    {
        Node* ast = new_node((strcmp(tokens[begin].content , "break") == 0) ? BREAK_node : CONTINUE_node,"");
        nodeptr_vector_push(&(ast->childs),new_node(line_node,int64_to_string(tokens[begin].ln)));
        return ast;
    }
    if(tokens[begin].type== ID_TOKEN)
    {
        if(tokens_size>=3)
        {
            if(tokens[begin+1].type== LParen_TOKEN && tokens[end].type== RParen_TOKEN && match_token(begin+1,end,RParen_TOKEN,tokens)==(int)end)
            {
                bool wrapInPrint = false;
                if(REPL_MODE && atGlobalLevel(ctx) && strcmp(tokens[begin].content,"print")!=0 && strcmp(tokens[begin].content,"println")!=0 && strcmp(tokens[begin].content,"printf")!=0)
                    wrapInPrint = true;
                Node* ast = new_node(call,"");
                Node* n = new_node(line_node,int64_to_string(tokens[begin].ln));
                nodeptr_vector_push(&(ast->childs),n);
                nodeptr_vector_push(&(ast->childs),new_node(ID_NODE,tokens[begin].content));
                bool done = false;
                for(size_t i = 1;i <= ctx->prefixes.size; i++)
                {
                    size_t idx = ctx->prefixes.size - i;
                    ctx->aux = merge_str(ctx->prefixes.arr[idx],tokens[begin].content);
                    if((done = addSymRef(ctx,ctx->aux)))
                    {
                        free((void*)ctx->aux);
                        break;
                    }
                    free((void*)ctx->aux);
                }
                Node* args = new_node(args_node,"");
                if(tokens_size==3)
                {
                    nodeptr_vector_push(&(ast->childs),args);  
                }
                else
                {
                    int args_begin = begin+2;
                    int args_end = end-1;
                    //
                    int arg_begin = args_begin;
                    int k;
                    for(k = args_begin;k<=args_end;k+=1)
                    {
                        if(tokens[k].type== COMMA_TOKEN)
                        {
                            int arg_end = k-1;
                            if(arg_begin > arg_end)
                                parseError(ctx,"SyntaxError","Invalid Syntax");
                            nodeptr_vector_push(&(args->childs),parseExpr(ctx,tokens,arg_begin,arg_end));
                            arg_begin = k+1;
                        }
                        else if(tokens[k].type== BEGIN_LIST_TOKEN)
                        {
                            int i = match_token(k,args_end,END_LIST_TOKEN,tokens);
                            if(i==-1)
                                parseError(ctx,"SyntaxError","Invalid Syntax");
                            //vector<token> P = {Args.begin()+k,Args.begin()+i+1};
                            //T.insert(T.end(),P.begin(),P.end());
                            k = i;
                        }
                        else if(tokens[k].type== L_CURLY_BRACKET_TOKEN)
                        {
                            int i = match_token(k,args_end,R_CURLY_BRACKET_TOKEN,tokens);
                            if(i==-1)
                                parseError(ctx,"SyntaxError","Invalid Syntax");
                            //vector<token> P = {Args.begin()+k,Args.begin()+i+1};
                            //T.insert(T.end(),P.begin(),P.end());
                            k = i;
                        }
                        else if(tokens[k].type== LParen_TOKEN)
                        {
                            int i = match_token(k,args_end,RParen_TOKEN,tokens);
                            if(i==-1)
                                parseError(ctx,"SyntaxError","Invalid Syntax");
                            //vector<token> P = {Args.begin()+k,Args.begin()+i+1};
                            //T.insert(T.end(),P.begin(),P.end());
                            k = i;
                        }
                }
                int arg_end = k-1;
                    if(arg_begin > arg_end)
                        parseError(ctx,"SyntaxError","Invalid Syntax");
                    nodeptr_vector_push(&(args->childs),parseExpr(ctx,tokens,arg_begin,arg_end));
                    nodeptr_vector_push(&(ast->childs),args);
                }
                if(wrapInPrint)
                {
                    Node* p = new_node(call,"");
                    Node* n = new_node(line_node,int64_to_string(tokens[begin].ln));
                    //p->childs.push_back(n);
                    add_child(p,n);
                    add_child(p,new_node(ID_NODE,"println"));
                    //p->childs.push_back();
                    Node* args = new_node(args_node,"");
                    nodeptr_vector_push(&(args->childs),ast);
                    //p->childs.push_back(args);
                    add_child(p, args);
                    return p;
                }
                return ast;
            }
        }
    }

    if(tokens[begin].type==KEYWORD_TOKEN && strcmp(tokens[begin].content,"class") == 0)
    {
        if(tokens_size<2 || tokens[begin+1].type != ID_TOKEN)
            parseError(ctx,"SyntaxError","Invalid Syntax");
        bool extendedClass=false;
        if(tokens_size >=4)
        {
            if(tokens[begin+2].type!=KEYWORD_TOKEN || tokens[begin+3].type!=ID_TOKEN)
                parseError(ctx,"SyntaxError","Invalid Syntax");
            if(strcmp(tokens[begin+2].content,"extends")!=0)
                parseError(ctx,"SyntaxError","Invalid Syntax");
            extendedClass = true;
        }
        else if(tokens_size==2)
        {

        }
        else
            parseError(ctx,"SyntaxError","Invalid Syntax");
        Node* ast;
        if(!extendedClass)
            ast = new_node(CLASS,"");
        else
            ast = new_node(EXTCLASS,"");
        Node* n = new_node(line_node,int64_to_string(ctx->line_num));
        nodeptr_vector_push(&(ast->childs),n);
        //Do not allow class names having '::'
        //if(((string)tokens[1].content).find("::")!=string::npos)
        //    parseError(ctx,"SyntaxError","Invalid Syntax");
        const char* fnprefix = ctx->prefixes.arr[ctx->prefixes.size-1];
        const char* tmp = tokens[begin+1].content;
        if(atGlobalLevel(ctx))
            tmp = merge_str(fnprefix,tokens[begin+1].content);
        nodeptr_vector_push(&(ast->childs),new_node(ID_NODE,tmp));
        if(extendedClass)
        {
            nodeptr_vector_push(&(ast->childs),parseExpr(ctx,tokens,begin+3,end));
        }
        return ast;
    }
    if(tokens[begin].type== KEYWORD_TOKEN && (strcmp(tokens[begin].content,"while")==0 || strcmp(tokens[begin].content,"dowhile") == 0))
    {
        if(tokens_size>= 4 && tokens[begin+1].type== LParen_TOKEN  && tokens[end].type== RParen_TOKEN)
        {
            Node* ast = new_node((strcmp(tokens[begin].content , "while") == 0) ? WHILE : DOWHILE,"");
            Node* n = new_node(line_node ,int64_to_string(tokens[begin].ln));
            nodeptr_vector_push(&(ast->childs),n);
            nodeptr_vector_push(&(ast->childs),parseExpr(ctx,tokens,begin+2,end-1));
            return ast;
        }   
    }
    if(tokens[begin].type==KEYWORD_TOKEN && strcmp(tokens[begin].content,"for") == 0)
    {
        if(tokens_size < 3 || tokens[begin+1].type!=LParen_TOKEN || tokens[end].type!=RParen_TOKEN)
            parseError(ctx,"SyntaxError","Invalid Syntax");
        
        begin += 2;
        end -= 1;
        tokens_size = end - begin+1;

        token t;
        t.type = KEYWORD_TOKEN;
        t.content = "to";
        int i = find_token(t,begin,end,tokens);
        bool dtoLoop = false;
        if(i==-1 || i > end)
        {
            t.content = "dto";
            i = find_token(t,begin,end,tokens);
            if(i == -1 || i > end)
                parseError(ctx,"SyntaxError","Invalid Syntax");
            dtoLoop = true;
        }
        int init_begin = begin;
        int init_end = i-1;
        int p = i;
        t.content = "step";
        t.type = KEYWORD_TOKEN;
        i = find_token(t,i+1,end,tokens);
        //vector<token> endpoint;
        //vector<token> inc;
        int endpoint_begin;
        int endpoint_end;
        token default_inc = make_token(NUM_TOKEN,"1", 0);
        int inc_begin;
        int inc_end;
        bool use_default_inc = true;
        if(i==-1)//step is optional
        {
            endpoint_begin = p+1;
            endpoint_end = end;
        }
        else
        {
            endpoint_begin = p+1;
            endpoint_end = i-1;
            inc_begin = i+1;
            inc_end = end;
            use_default_inc = false;
        }
        bool decl = false;
        if(tokens[init_begin].type==KEYWORD_TOKEN && strcmp(tokens[init_begin].content,"var")==0 )
        {
            init_begin++;
            decl = true;
        }
        if(tokens[init_begin].type==ID_TOKEN && tokens[init_begin+1].type==OP_TOKEN && strcmp(tokens[init_begin+1].content,"=") == 0)
        {
            int init_expr_begin = init_begin+2;
            int init_expr_end = init_end;
            Node* ast;
            if(dtoLoop)
                ast = new_node(DFOR,"");
            else
                ast = new_node(FOR,"");
            Node* n = new_node(line_node,int64_to_string(tokens[begin].ln));
            nodeptr_vector_push(&(ast->childs),n);
            if(decl)
                nodeptr_vector_push(&(ast->childs),new_node(decl_node,""));
            else
                nodeptr_vector_push(&(ast->childs),new_node(nodecl,""));

            nodeptr_vector_push(&(ast->childs),new_node(ID_NODE,tokens[init_begin].content));
            nodeptr_vector_push(&(ast->childs),parseExpr(ctx,tokens,init_expr_begin,init_expr_end));
            nodeptr_vector_push(&(ast->childs),parseExpr(ctx,tokens,endpoint_begin,endpoint_end));
            if(use_default_inc)
                nodeptr_vector_push(&(ast->childs),parseExpr(ctx,&default_inc,0,0));
            else
                nodeptr_vector_push(&(ast->childs),parseExpr(ctx,tokens,inc_begin,inc_end));
            return ast;
        }
        else
            parseError(ctx,"SyntaxError","Invalid Syntax");
    }
    if(tokens[begin].type==KEYWORD_TOKEN && strcmp(tokens[begin].content,"foreach") == 0)
    {
        if(tokens_size<3)
            parseError(ctx,"SyntaxError","Invalid Syntax");

        if(tokens[begin+1].type!=LParen_TOKEN || tokens[end].type!=RParen_TOKEN)
            parseError(ctx,"SyntaxError","Invalid Syntax");
        begin += 2;
        end -= 1;
        tokens_size = end - begin+1;
        if(tokens_size<4)
            parseError(ctx,"SyntaxError","Invalid Syntax");
        if(tokens[begin].type!=KEYWORD_TOKEN || strcmp(tokens[begin].content,"var")!=0 || tokens[begin+1].type!=ID_TOKEN || tokens[begin+2].type!=COLON_TOKEN)
            parseError(ctx,"SyntaxError","Invalid Syntax");
      
        Node* exprAST = parseExpr(ctx,tokens,begin+3,end);
        Node* ast = new_node(FOREACH,"");
        Node* n = new_node(line_node,int64_to_string(tokens[begin].ln));
        //ast->childs = {n,,exprAST};
        add_child(ast,n);
        add_child(ast,new_node(ID_NODE,tokens[begin+1].content));
        add_child(ast, exprAST);
        return ast;
    }
    if(tokens[begin].type==KEYWORD_TOKEN && strcmp(tokens[begin].content,"namespace") == 0)
    {
        if(tokens_size!=2)
            parseError(ctx,"SyntaxError","Invalid Syntax");
        if(tokens[begin+1].type!=ID_TOKEN)
            parseError(ctx,"SyntaxError","Invalid Syntax");
        Node* ast = new_node(NAMESPACE,"");
        Node* n = new_node(line_node,int64_to_string(tokens[begin].ln));
        nodeptr_vector_push(&(ast->childs),n);
        //string fnprefix;
        //if(((string)tokens[1].content).find("::")!=string::npos)
        //    parseError(ctx,"SyntaxError","Invalid Syntax");

        nodeptr_vector_push(&(ast->childs),new_node(ID_NODE,tokens[begin+1].content));
        return ast;
    }
    if(tokens[begin].type== KEYWORD_TOKEN && strcmp(tokens[begin].content,"if") == 0)
    {
        if(tokens_size>=4)
        {
            if(tokens[begin+1].type== LParen_TOKEN  && tokens[end].type== RParen_TOKEN)
            {
                Node* ast = new_node(IF,"");
                Node* n = new_node(line_node,int64_to_string(tokens[begin].ln));
                nodeptr_vector_push(&(ast->childs),n);
                Node* conditions = new_node(conditions_node,"");
                add_child(conditions,parseExpr(ctx,tokens,begin+2,end-1));
                add_child(ast,conditions);
                return ast;
            }
        }
    }
    if(tokens[begin].type== KEYWORD_TOKEN && strcmp(tokens[begin].content,"import") == 0)
    {
        Node* ast = new_node(import,"");
        if(tokens_size==2)
        {
            if(tokens[begin+1].type==ID_TOKEN)
            {
                Node* n = new_node(line_node,int64_to_string(tokens[begin].ln));
                nodeptr_vector_push(&(ast->childs),n);
                nodeptr_vector_push(&(ast->childs),parseExpr(ctx,tokens,begin+1,begin+1));
                return ast;
            }
            if(tokens[begin+1].type== STRING_TOKEN)
            {
                Node* n = new_node(line_node,int64_to_string(tokens[begin].ln));
                nodeptr_vector_push(&(ast->childs),n);
                nodeptr_vector_push(&(ast->childs),new_node(STR_NODE,tokens[begin+1].content));
                return ast;
            }
            else
            {
                    //line_num = tokens[1].ln;
                    parseError(ctx,"SyntaxError","Invalid Syntax");
            }
        }
        else if(tokens_size==4 && strcmp(tokens[begin+2].content , "as") == 0)
        {
            if(tokens[begin+1].type!=ID_TOKEN || tokens[begin+2].type!=KEYWORD_TOKEN || strcmp(tokens[begin+2].content,"as")!=0 || tokens[begin+3].type!=ID_TOKEN)
            {
                parseError(ctx,"SyntaxError","Invalid Syntax");
            }
            ast->type = importas;

            Node* n = new_node(line_node,int64_to_string(tokens[begin].ln));
            nodeptr_vector_push(&(ast->childs),n);
            nodeptr_vector_push(&(ast->childs),parseExpr(ctx,tokens,begin+1,begin+1));
            nodeptr_vector_push(&(ast->childs),new_node(ID_NODE,tokens[begin+3].content));
            return ast;
        }
        else if(tokens_size==4)
        {
            if(tokens[begin+1].type!=ID_TOKEN || strcmp(tokens[begin+1].content,"std")!=0 || tokens[begin+2].type!=OP_TOKEN || strcmp(tokens[begin+2].content,"/")!=0 || tokens[begin+3].type!=ID_TOKEN )
                parseError(ctx,"SyntaxError","Invalid Syntax");
            char* tmp;
            #ifdef _WIN32
            tokens[begin+3].content="C:\\zuko\\std\\"+tokens[begin+3].content+".zk";
            #else
            tmp = merge_str(merge_str("/opt/zuko/std/",tokens[begin+3].content),".zk");
            #endif
            Node* n = new_node(line_node,int64_to_string(tokens[begin].ln));
            nodeptr_vector_push(&(ast->childs),n);
            nodeptr_vector_push(&(ast->childs),new_node(STR_NODE,tmp));
            return ast;
        }
        else
        {
            parseError(ctx,"SyntaxError","Invalid Syntax");
        }
    }
    if(tokens[begin].type== KEYWORD_TOKEN && strcmp(tokens[begin].content,"return") == 0)
    {
        Node* ast = new_node(RETURN_NODE,"");
        int expr_begin = begin+1;
        int expr_end = end;
        if(expr_begin > expr_end)
            parseError(ctx,"SyntaxError","Error expected expression after return keyword");
        Node* n = new_node(line_node,int64_to_string(tokens[begin].ln));
        nodeptr_vector_push(&(ast->childs),n);
        nodeptr_vector_push(&(ast->childs),parseExpr(ctx,tokens,expr_begin,expr_end));
        return ast;
    }
    if(tokens[begin].type== KEYWORD_TOKEN && strcmp(tokens[begin].content,"yield") == 0)
    {
        ctx->foundYield = true;
        Node* ast = new_node(YIELD_node,"");
        if(begin+1 > end)
            parseError(ctx,"SyntaxError","Error expected expression after yield keyword");
        Node* n = new_node(line_node,int64_to_string(tokens[0].ln));
        nodeptr_vector_push(&(ast->childs),n);
        nodeptr_vector_push(&(ast->childs),parseExpr(ctx,tokens,begin+1,end));
        return ast;
    }
    if(tokens[begin].type== KEYWORD_TOKEN && strcmp(tokens[begin].content,"function") == 0)
    {
        if(tokens_size<4)
            parseError(ctx,"SyntaxError","Invalid Syntax");
        
        if(tokens[begin+1].type!=ID_TOKEN || tokens[end].type!=RParen_TOKEN)
            parseError(ctx,"SyntaxError","Invalid Syntax");
        //if(((string)tokens[begin+1].content).find("::")!=string::npos)
        //    parseError(ctx,"SyntaxError","Invalid Syntax");
        const char* fnprefix = ctx->prefixes.arr[ctx->prefixes.size-1];
        const char* tmp = tokens[begin+1].content;
        if(atGlobalLevel(ctx))
            tmp = merge_str(fnprefix,tokens[begin+1].content);
        if(isPrivate)
            tmp = merge_str("@" , tokens[begin+1].content);
        Node* ast = new_node(FUNC,"");
        Node* n = new_node(line_node,int64_to_string(tokens[begin].ln));
        nodeptr_vector_push(&(ast->childs),n);
        nodeptr_vector_push(&(ast->childs),new_node(ID_NODE,tmp));
        nodeptr_vector_push(&(ast->childs),new_node(args_node,""));
        if(tokens_size==4)
            return ast;
        int args_begin = begin+3;
        int args_end = end-1;
        if(args_end - args_begin+1 < 2)
            parseError(ctx,"SyntaxError","Invalid Syntax");
        
        size_t k = args_begin;
        bool found_default = false;//found any default argument
        //string tname;
        while(k<=args_end)
        {
            if((tokens[k].type== KEYWORD_TOKEN && strcmp(tokens[k].content,"var") == 0))
            {
                if(k==args_end)
                    parseError(ctx,"SyntaxError","Invalid Syntax");
                if(tokens[k+1].type!= ID_TOKEN)
                    parseError(ctx,"SyntaxError","Invalid Syntax");
                //ast->childs[2]->childs.push_back();
                add_child(ast->childs.arr[2],new_node(ID_NODE,tokens[k+1].content));
                k+=1;
                if(k<args_end)// or k+1 < args.size() or in simple English(there are more parameters)
                {
                    k+=1;//it must be a comma
                    if(tokens[k].type==OP_TOKEN && strcmp(tokens[k].content,"=")==0)//default parameters
                    {
                        k+=1;
                        if(k > args_end)
                            parseError(ctx,"SyntaxError","Invalid Syntax");
                        int j = k;
                        int i1 = 0;
                        int i2 = 0;
                        int i3 = 0;
                        bool found = false;
                        for(;j<=args_end;j++)
                        {
                        if(tokens[j].type==L_CURLY_BRACKET_TOKEN)
                            i1+=1;
                        else if(tokens[j].type==R_CURLY_BRACKET_TOKEN)
                            i1-=1;
                        else if(tokens[j].type==BEGIN_LIST_TOKEN)
                            i2+=1;
                        else if(tokens[j].type==END_LIST_TOKEN)
                            i2-=1;
                        else if(tokens[j].type==LParen_TOKEN)
                            i3+=1;
                        else if(tokens[j].type==RParen_TOKEN)
                            i3-=1;
                        else if(tokens[j].type==COMMA_TOKEN && i1==0 && i2==0 && i3==0)
                        {
                            found = true;
                            break;
                        }
                        }
                        if(!found && (size_t)j!=args_end+1)
                            parseError(ctx,"SyntaxError","Invalid Syntax");
                        found_default = true;
                        //ast->childs[2]->childs.back()->childs.push_back();
                        Node* tmp = ast->childs.arr[2]->childs.arr[ast->childs.arr[2]->childs.size-1];
                        add_child(tmp,parseExpr(ctx,tokens,k,j-1));
                        k = j+1;
                        continue;
                    }
                    if(found_default)
                        parseError(ctx,"SyntaxError","Invalid Syntax");
                    if(tokens[k].type!= COMMA_TOKEN)
                        parseError(ctx,"SyntaxError","Invalid Syntax");
                    if(k==args_end)
                        parseError(ctx,"SyntaxError","Invalid Syntax");
                    k+=1;
                    continue;
                }
            }
            else
                parseError(ctx,"SyntaxError","Invalid Syntax");
            if(found_default)
                parseError(ctx,"SyntaxError","Invalid Syntax");
            k+=1;
        }
        return ast;
    }
    int k=begin;
    while(k<=end)
    {
        if(tokens[k].type!= OP_TOKEN)
        {
            k+=1;
            continue;
        }
        if(strcmp(tokens[k].content,"=")==0)
        {
            Node* ast = new_node(assign,tokens[k].content);
            //vector<token> left = {tokens.begin()+begin,tokens.begin()+k};
            //if(left.size()==0)
            //    parseError(ctx,"SyntaxError","Invalid Syntax");
            Node* n = new_node(line_node,int64_to_string(tokens[k].ln));
            nodeptr_vector_push(&(ast->childs),n);
            nodeptr_vector_push(&(ast->childs),parseExpr(ctx,tokens,begin,k-1));
            int rhs_begin = k+1;
            int rhs_end = end;
            if(rhs_begin > rhs_end)
                parseError(ctx,"SyntaxError","Invalid Syntax");
            if(tokens[rhs_begin].type==KEYWORD_TOKEN && strcmp(tokens[rhs_begin].content,"yield")==0)
            {
                rhs_begin++;
                nodeptr_vector_push(&(ast->childs),new_node(YIELD_node,""));
                if(rhs_begin > rhs_end)
                    parseError(ctx,"SyntaxError","Error expected expression after return keyword");
                Node* n = new_node(line_node,int64_to_string(tokens[k+1].ln));
                //ast->childs.back()->childs.push_back(n);
                add_child(ast->childs.arr[ast->childs.size-1],n);
//                ast->childs.back()->childs.push_back(parseExpr(ctx,0,rhs_begin,rhs_end));
                add_child(ast->childs.arr[ast->childs.size-1],parseExpr(ctx,tokens,rhs_begin,rhs_end));
                ctx->foundYield = true;
                return ast;
            }
            nodeptr_vector_push(&(ast->childs),parseExpr(ctx,tokens,rhs_begin,rhs_end));
            return ast;
        }
        else if(strcmp(tokens[k].content,"+=")==0 || strcmp(tokens[k].content,"-=")==0 || strcmp(tokens[k].content,"/=")==0 || strcmp(tokens[k].content,"*=")==0 || strcmp(tokens[k].content,"^=")==0 || strcmp(tokens[k].content,"%=")==0 || strcmp(tokens[k].content,"|=")==0 || strcmp(tokens[k].content,"&=")==0 || strcmp(tokens[k].content,"<<=")==0 || strcmp(tokens[k].content,">>=")==0)
        {
            Node* ast = new_node(assign,"");
            int lhs_begin = begin;
            int lhs_end = k-1;
            ctx->line_num  = tokens[k].ln;
            if(lhs_begin > lhs_end)
                parseError(ctx,"SyntaxError","Invalid Syntax");

            Node* n = new_node(line_node,int64_to_string(tokens[k].ln));
            nodeptr_vector_push(&(ast->childs),n);
            nodeptr_vector_push(&(ast->childs),parseExpr(ctx,tokens,begin,k-1)); // lhs
            
            char tmp[2] = {tokens[k].content[0],0};
            const char* copy = tokens[k].content;
            tokens[k].content = tmp;

            if(k+1 > end)
                parseError(ctx,"SyntaxError","Invalid Syntax");
            nodeptr_vector_push(&(ast->childs),parseExpr(ctx,tokens,lhs_begin,end));
            tokens[k].content = copy;
            return ast;
        }
        k+=1;
    }
    
    
    if(tokens[begin].type==KEYWORD_TOKEN && strcmp(tokens[begin].content,"try") == 0)
    {
        Node* line = new_node(line_node,int64_to_string(tokens[begin].ln));
        Node* ast = new_node(TRYCATCH,"");
        nodeptr_vector_push(&(ast->childs),line);
        return ast;
    }
    if(tokens[begin].type==KEYWORD_TOKEN && strcmp(tokens[begin].content,"throw") == 0)
    {
        if(tokens_size < 2)
            parseError(ctx,"SyntaxError","Invalid Syntax");
        Node* line = new_node(line_node ,int64_to_string(tokens[begin].ln));
        Node* ast = new_node(THROW_node,"");
        nodeptr_vector_push(&(ast->childs),line);
        nodeptr_vector_push(&(ast->childs),parseExpr(ctx,tokens,begin+1,end));
        return ast;
    }
    //Handle statements of the form
    //expr.fun()
    k = end;
    size_t iterator = begin; 
    while(iterator<=end)
    {
        if(tokens[k].type==RParen_TOKEN)
        {
            int i = match_token_right(k,begin,LParen_TOKEN,tokens);
            if(i==-1)
                parseError(ctx,"SyntaxError","Invalid Syntax");
            k = i;
        }
        else if(tokens[k].type== END_LIST_TOKEN)
        {
            int i = match_token_right(k,begin,BEGIN_LIST_TOKEN,tokens);
            if(i==-1)
                parseError(ctx,"SyntaxError","Invalid Syntax");
            k=i;
        }
        else if(tokens[k].type== R_CURLY_BRACKET_TOKEN)
        {
            int i = match_token_right(k,begin,L_CURLY_BRACKET_TOKEN,tokens);
            if(i==-1)
                parseError(ctx,"SyntaxError","Invalid Syntax");
            k=i;
        }
        else if(tokens[k].type== OP_TOKEN && strcmp(tokens[k].content,".")==0)
        {
            Node* ast = new_node(memb,"");
            Node* line = new_node(line_node,int64_to_string(tokens[0].ln));
            nodeptr_vector_push(&(ast->childs),line);
           
            nodeptr_vector_push(&(ast->childs),parseExpr(ctx,tokens,begin,k-1));
            nodeptr_vector_push(&(ast->childs),parseExpr(ctx,tokens,k+1,end));
            //rhs must be a function call as stated above

            if(ast->childs.arr[ast->childs.size-1]->type!=call)
            {
                delete_ast(ast);
                break;
            }
            return ast;
        }
        iterator+=1;
        k-=1;
    }
    if(REPL_MODE)
    {
        token_vector dummyStmt;
        token_vector_init(&dummyStmt);
        token_vector_push(&dummyStmt,make_token(ID_TOKEN,"println",tokens[0].ln));
        token_vector_push(&dummyStmt,make_token(LParen_TOKEN,"(",0));
        for(int i=begin;i<=end;i++)
            token_vector_push(&dummyStmt,tokens[k]);
        token_vector_push(&dummyStmt,make_token(RParen_TOKEN,")",0));
        Node* ast = parseExpr(ctx,dummyStmt.arr,0,dummyStmt.size-1);
        free(dummyStmt.arr);
        return ast;
    }
    parseError(ctx,"SyntaxError","Unknown statement");
    return NULL;//to avoid compiler warning otherwise the parseError function exits after printing error message
}

token else_token;
token elif_token;
token newline_token;
token bgscope_token;
token endscope_token;
token lp_token;
typedef struct pair
{
    int x;
    int y;
}pair;
Node* parse_block(Parser* ctx,token* tokens,int begin,int end)
{
    Node* ast = NULL;
    else_token = make_token(KEYWORD_TOKEN, "else", 0);
    elif_token = make_token(KEYWORD_TOKEN, "else if", 0);
    newline_token = make_token(NEWLINE_TOKEN, "\n", 0);
    bgscope_token = make_token(L_CURLY_BRACKET_TOKEN, "{", 0);
    endscope_token = make_token(R_CURLY_BRACKET_TOKEN, "}", 0);
    lp_token = make_token(LParen_TOKEN, "(", 0);

    strip_newlines(tokens, &begin, &end);

    size_t tokens_size = end - begin + 1;
    if(tokens_size == 0 || begin > end)
        return new_node(EOP,"");

    Node* final = NULL; // the final ast to return
    Node* e = NULL;//e points to the lowest rightmost node of final
    ctx->line_num = 1;
    bool a,b,c;


    size_t k = begin;
    token_vector multiline;
    token_vector_init(&multiline);
    while(k<=end)
    {
        if(tokens[k].type== NEWLINE_TOKEN || k == end)
        {
            if(k == end)
                k++;
            size_t line_begin = begin;
            size_t line_end = k-1;
            size_t line_size = line_end - line_begin + 1;

            if(line_begin > line_end)
            {
                begin += 1;
                k+=1;
                continue;
            }
            ctx->line_num = tokens[line_begin].ln;
            //for ifelse and loop statements, we pop the curly bracket from line
            //for multiline expressions we don't
            //multiline expressions begin by adding '{' or '(' or '[' at the end
            //of a line that is not a conditional statement or loop stmt
            bool use_multiline = false;
            if
            (
                ((a = tokens[line_end].type == L_CURLY_BRACKET_TOKEN) ||
                (b = tokens[line_end].type == LParen_TOKEN) ||
                (c = tokens[line_end].type == BEGIN_LIST_TOKEN)) && line_size != 1
            )
            {
                if( tokens[line_begin].type != KEYWORD_TOKEN ||
                    (tokens[line_begin].type == KEYWORD_TOKEN &&
                    (strcmp(tokens[line_begin].content , "var") == 0 || strcmp(tokens[line_begin].content,"return")==0 || strcmp(tokens[line_begin].content,"yield") == 0)
                    )   
                )
                {
                    int idx = (int)k -1;
                    int rp = 0;
                    if(a)
                        rp = match_token(idx,end,R_CURLY_BRACKET_TOKEN,tokens);
                    else if(b)
                        rp = match_token(idx,end,RParen_TOKEN,tokens);
                    else if(c)
                        rp = match_token(idx,end,END_LIST_TOKEN,tokens);
                    if(rp == -1)
                        parseError(ctx,"SyntaxError","Invalid Syntax");
                    

                    size_t i = 0;
                    line_end = rp;
                    multiline.size = 0;
                    for(i=begin;i<=(size_t)rp ;i++)
                    {
                        if(tokens[i].type != NEWLINE_TOKEN)
                            token_vector_push(&multiline,tokens[i]);
                    }
                    //rp currently includes ending bracket on some line
                    //that whole line should be included
                    rp++;
                    while(rp <= end )
                    {
                        if(tokens[rp].type == NEWLINE_TOKEN)
                            break;
                        else
                            token_vector_push(&multiline,tokens[rp]);
                        rp+=1;
                        line_end++;
                    }
                    use_multiline = true;
                    line_begin = 0;
                    line_end = multiline.size-1;
                    k = rp;//tokens before ith idx are handled
                }
                else if(a)//pop curly bracket only
                    line_end -=1 ;
                
            }

            line_size = line_end - line_begin +1;

            if(use_multiline)
            {
                ctx->line_num = multiline.arr[0].ln;
                ast = parseStmt(ctx,multiline.arr, line_begin,line_end);
            }
            else
            {
                ctx->line_num = tokens[line_begin].ln;
                ast = parseStmt(ctx,tokens,line_begin,line_end);
            }

            if(ast->type==IF)
            {
                token lptok;
                lptok.type = LParen_TOKEN;
                lptok.content = "(";
                int if_begin = find_token_consecutive(bgscope_token,begin+line_size,end,tokens);
                int if_end;
                if(if_begin!=-1)
                {
                    if_end = match_token(if_begin,end,R_CURLY_BRACKET_TOKEN,tokens);
                    if(if_end == -1)
                    {
                      ctx->line_num = tokens[if_begin].ln;
                      parseError(ctx,"SyntaxError","Error expected '}' to match this.");
                    }
                }
                else
                {
                    if_begin = find_token_consecutive(newline_token,begin+line_size,end,tokens);
                    if_end = find_token(newline_token,if_begin+1,end,tokens);
                    ctx->line_num = tokens[line_begin].ln;
                    if(if_end==-1)
                        if_end = end+1;
                }

                if_begin += 1;
                if_end -= 1;

                pair* elifBlocks = NULL;
                pair* elifConditions = NULL;
                int elif_size = 0;
                int elif_begin = find_token_consecutive(elif_token,if_end+2,end,tokens);
                int elif_end = -1;
                bool foundelif = false;
                bool foundElse = false;
                if(elif_begin!=-1)
                    foundelif = true;
                int last_elif_end = -1;
                while(elif_begin!=-1)
                {
                    int p = find_token_consecutive(lptok,elif_begin+1,end,tokens);
                    if(p==-1 || p!=elif_begin+1)
                    {
                        ctx->line_num = tokens[elif_begin].ln;
                        parseError(ctx,"SyntaxError","Error expected '(' after else if");
                    }
                    int l = match_token(p,end,RParen_TOKEN,tokens);
                    elifConditions = realloc(elifConditions,sizeof(pair)*(elif_size + 1));
                    elifConditions[elif_size].x = p+1;
                    elifConditions[elif_size].y = l-1;

                    elif_begin = find_token_consecutive(bgscope_token,l+1,end,tokens);
                    if(elif_begin==-1)
                    {
                        elif_begin = find_token_consecutive(newline_token,l+1,end,tokens);
                        elif_end = find_token(newline_token,elif_begin+1,end,tokens);
                        ctx->line_num = tokens[p].ln;
                        elif_begin-=1;
                        if(elif_end == -1)
                            elif_end = end+1;

                    }
                    else
                        elif_end = match_token(elif_begin,end,R_CURLY_BRACKET_TOKEN,tokens);//IMPORTANT
                    
//                    elifBlocks.push_back(std::pair<int,int>(elif_begin+1,elif_end-1));
                    elifBlocks = realloc(elifBlocks,sizeof(pair)*(elif_size+1));
                    elifBlocks[elif_size].x = elif_begin+1;
                    elifBlocks[elif_size].y = elif_end-1;
                    elif_begin = find_token_consecutive(elif_token,elif_end+1,end,tokens);
                    last_elif_end = elif_end-1;
                    elif_size++;
                }
                int else_begin = -1;
                if(foundelif)
                    else_begin = find_token_consecutive(else_token,last_elif_end+2,end,tokens);
                else
                    else_begin = find_token_consecutive(else_token,if_end+2,end,tokens);
                int else_end = -1;

                if(else_begin!=-1)
                {
                    foundElse= true;
                    ctx->line_num = tokens[else_begin].ln;
                    int e = else_begin;
                    else_begin = find_token_consecutive(bgscope_token,else_begin+1,end,tokens);
                    if(else_begin==-1)
                    {
                        else_begin = find_token_consecutive(newline_token,e+1,end,tokens);
                        if(else_begin == -1)
                            parseError(ctx,"SyntaxError","Expected statement after else keyword");
                        else_end = find_token(newline_token,else_begin+1,end,tokens);
                        if(else_end == -1)
                            else_end = end+1;

                    }
                    else
                        else_end = match_token(else_begin,end,R_CURLY_BRACKET_TOKEN,tokens);//IMPORTANT
                    else_begin += 1;
                    else_end -= 1;
                }
                if(foundelif && foundElse)
                {
                    ast->type = IFELIFELSE;
                    for(int a=0;a<(int)elif_size;a++)
                    {
                        ctx->line_num = tokens[elifConditions[a].x].ln;
                        add_child(ast->childs.arr[1],parseExpr(ctx,tokens,elifConditions[a].x,elifConditions[a].y));
                    }

                    bool ctxCopy = ctx->inif;
                    ctx->inif = true;
                    nodeptr_vector_push(&(ast->childs),parse_block(ctx,tokens,if_begin,if_end));
                    ctx->inif = ctxCopy;

                    for(int a=0;a<(int)elif_size;a++)
                    {
                        int elif_begin = elifBlocks[a].x;
                        int elif_end = elifBlocks[a].y;
                        bool ctxCopy = ctx->inelif;
                        ctx->inelif = true;
                        Node* n = parse_block(ctx,tokens,elif_begin,elif_end);
                        ctx->inelif = ctxCopy;
                        nodeptr_vector_push(&(ast->childs),n);
                    }
                    ctxCopy = ctx->inelse;
                    ctx->inelse = true;
                    nodeptr_vector_push(&(ast->childs),parse_block(ctx,tokens,else_begin,else_end));
                    ctx->inelse = ctxCopy;
                    begin = else_end+2;
                    k = else_end+1;
                }
                else if(foundelif && !foundElse)
                {
                    ast->type = IFELIF;
                    for(int a=0;a<(int)elif_size;a++)
                    {
                        //vector<token> cond = {tokens.begin()+elifConditions[a].first,tokens.begin()+elifConditions[a].second+1};
                        size_t cond_begin = elifConditions[a].x;
                        size_t cond_end = elifConditions[a].y;
                        ctx->line_num = tokens[cond_begin].ln;
                        add_child(ast->childs.arr[1],parseExpr(ctx,tokens,cond_begin,cond_end));
                    }
                    bool ctxCopy = ctx->inif;
                    ctx->inif = true;
                    nodeptr_vector_push(&(ast->childs),parse_block(ctx,tokens,if_begin,if_end));
                    ctx->inif = ctxCopy;

                    for(int a=0;a<(int)elif_size;a++)
                    {
                        int elifblock_begin = elifBlocks[a].x;
                        int elifblock_end = elifBlocks[a].y;
                        bool ctxCopy = ctx->inelif;
                        ctx->inelif = true;
                        Node* n = parse_block(ctx,tokens,elifblock_begin,elifblock_end);
                        ctx->inelif = ctxCopy;
                        nodeptr_vector_push(&(ast->childs),n);
                    }
                    begin=last_elif_end+2;
                    k = last_elif_end+1;
                }
                else if(!foundelif && foundElse)
                {
                    ast->type = IFELSE;
                    bool ctxCopy = ctx->inif;
                    ctx->inif = true;

                    nodeptr_vector_push(&(ast->childs),parse_block(ctx,tokens,if_begin,if_end));
                    ctx->inif = ctxCopy;
                    ctxCopy = ctx->inelse;
                    ctx->inelse = true;

                    nodeptr_vector_push(&(ast->childs),parse_block(ctx,tokens,else_begin,else_end));
                    ctx->inelse = ctxCopy;
                    
                    begin=else_end+2;
                    k = else_end+1;
                }
                else if(!foundelif && !foundElse)
                {
                    ast->type = IF;
                    bool ctxCopy = ctx->inif;
                    ctx->inif = true;
                    nodeptr_vector_push(&(ast->childs),parse_block(ctx,tokens,if_begin,if_end));
                    ctx->inif = ctxCopy;
                    
                    begin=if_end+2;
                    k = if_end+1;
                }
            }
            else if(ast->type==FUNC)
            {
                int func_begin = find_token_consecutive(bgscope_token,begin+line_size,end,tokens);
                int func_end = -1;
                if(func_begin==-1)
                {
                    func_begin = find_token_consecutive(newline_token,begin+line_size,end,tokens);
                    if(func_begin==-1)
                        parseError(ctx,"SyntaxError","Expected code block after function definition!");
                    func_end = find_token(newline_token,func_begin+1,end,tokens);
                    if(func_end==-1)
                        parseError(ctx,"SyntaxError","Expected code block after function definition!");
                }
                else
                    func_end = match_token(func_begin,end,R_CURLY_BRACKET_TOKEN,tokens);
                if(func_end==-1)
                    parseError(ctx,"SyntaxError","Error missing '}' to end function definition.");
                if(func_begin==-1)
                    parseError(ctx,"SyntaxError","Error expected code block or line after function definition!");
                func_begin += 1;
                func_end -= 1;

                if(ctx->infunc)
                    parseError(ctx,"SyntaxError","Nested functions not allowed!");
                if(!isValidCtxForFunc(ctx))
                    parseError(ctx,"SyntaxError","Local functions not allowed!");
                ctx->foundYield = false;
                const char* aux = ctx->currSym;
                if(!ctx->inclass)
                {
                    ctx->currSym = ast->childs.arr[1]->val;
                    str_vector v;
                    str_vector_init(&v);
                    refgraph_emplace(ctx->refGraph,clone_str(ast->childs.arr[1]->val),v);
                }
                ctx-> infunc = true;
                
                nodeptr_vector_push(&(ast->childs),parse_block(ctx,tokens,func_begin,func_end));
                ctx->infunc = false;
                if(!ctx->inclass)
                    ctx->currSym = (aux);

                if(ctx->foundYield)
                    ast->type = CORO;


                begin=func_end+2;
                k = func_end+1;
            }
            else if(ast->type==CLASS || ast->type==EXTCLASS)
            {
                int class_begin = find_token_consecutive(bgscope_token,begin+line_size,end,tokens);
                int class_end = -1;
                if(class_begin==-1)
                {
                    parseError(ctx,"SyntaxError","Error expected brackets {} after class definition!");
                }
                else
                    class_end = match_token(class_begin,end,R_CURLY_BRACKET_TOKEN,tokens);
                if(!atGlobalLevel(ctx))
                    parseError(ctx,"SyntaxError","Class declartions must be at global scope or in a namespace!");
                class_begin += 1;
                class_end -= 1;
                ctx->inclass = true;
                const char* aux = ctx->currSym;
                ctx->currSym = ast->childs.arr[1]->val;

                str_vector v;
                str_vector_init(&v);
                refgraph_emplace(ctx->refGraph,clone_str(ast->childs.arr[1]->val),v);
                
                if(ast->type == CLASS)
                    nodeptr_vector_push(&(ast->childs),parse_block(ctx,tokens,class_begin,class_end));
                else
                    nodeptr_vector_push(&(ast->childs),parse_block(ctx,tokens,class_begin,class_end));
                
                //backtrack
                //important: change if planning to support nested classes
                ctx->inclass = false;
                ctx->currSym = clone_str(aux);
                
                begin=class_end+2;
                k = class_end+1;
            }
            else if(ast->type==NAMESPACE)
            {
                int nm_begin = find_token_consecutive(bgscope_token,begin+line_size,end,tokens);
                int nm_end = -1;
                if(nm_begin==-1)
                {
                    ctx->line_num = tokens[begin].ln;
                    parseError(ctx,"SyntaxError","Error expected {} after namespace keyword");
                }
                else
                    nm_end = match_token(nm_begin,end,R_CURLY_BRACKET_TOKEN,tokens);
                if(!atGlobalLevel(ctx))
                    parseError(ctx,"SyntaxError","Namespace declaration must be at global scope or within another namespace!");
                //vector<token> block = {tokens.begin()+j+1,tokens.begin()+i};
                nm_begin += 1;
                nm_end -= 1;

                const char* prefix = "";
                for(size_t i = 0;i<ctx->prefixes.size;i++)
                {
                    const char* e = ctx->prefixes.arr[i];
                    prefix = merge_str(prefix,e);
                }
                prefix = merge_str(prefix,ast->childs.arr[1]->val);
                prefix = merge_str(prefix,"::");
                str_vector_push(&ctx->prefixes,prefix);
                //with "namespaceName::"
                nodeptr_vector_push(&(ast->childs),parse_block(ctx,tokens,nm_begin,nm_end));
                char* tmp;
                str_vector_pop(&ctx->prefixes,&tmp);
            
                begin = nm_end+2;
                k = nm_end+1;
            }
            else if(ast->type==WHILE || ast->type==DOWHILE || ast->type==FOR || ast->type == DFOR || ast->type==FOREACH)//loops
            {
                int loop_begin = find_token_consecutive(bgscope_token,begin+line_size,end,tokens);
                int loop_end;
                if(loop_begin==-1)
                {
                    loop_begin = find_token_consecutive(newline_token,begin+line_size,end,tokens);
                    loop_end = find_token(newline_token,loop_begin+1,end,tokens);
                    if(loop_end==-1)
                        loop_end = end;
                }
                else
                    loop_end = match_token(loop_begin,end,R_CURLY_BRACKET_TOKEN,tokens);
                
                loop_begin += 1;
                loop_end -= 1;
                bool b = ctx->inloop;
                ctx->inloop = true;
                nodeptr_vector_push(&(ast->childs),parse_block(ctx,tokens,loop_begin,loop_end));//inwhile = true
                ctx->inloop = b;
                
                begin=loop_end+2;
                k = loop_end+1;
            }
            else if(ast->type==TRYCATCH)//try catch
            {
                int try_begin = find_token_consecutive(bgscope_token,begin+line_size,end,tokens);
                int try_end = -1;
                if(try_begin==-1)
                {
                    try_begin = find_token_consecutive(newline_token,begin+line_size,end,tokens);
                    try_end = find_token(newline_token,try_begin+1,end,tokens);
                    if(try_end==-1)
                        parseError(ctx,"SyntaxError","Error expected a code block or line after try!");
                }
                else
                    try_end = match_token(try_begin,end,R_CURLY_BRACKET_TOKEN,tokens);

                try_begin += 1;
                try_end -= 1;

                token CATCH_TOK;
                CATCH_TOK.type = KEYWORD_TOKEN;
                CATCH_TOK.content = "catch";
                const char* catchId ;
                int catch_begin = find_token_consecutive(CATCH_TOK,try_end+2,end,tokens);
                int catch_end = -1;
                if(catch_begin!=-1)
                {
                    ctx->line_num = tokens[catch_begin].ln;
                    if(catch_begin+4 > end)
                        parseError(ctx,"SyntaxError","Invalid Syntax");
                    catch_begin+=1;
                    if(tokens[catch_begin].type!=LParen_TOKEN)
                        parseError(ctx,"SyntaxError","Invalid Syntax");
                    catch_begin+=1;
                    if(tokens[catch_begin].type!=ID_TOKEN)
                        parseError(ctx,"SyntaxError","Invalid Syntax");
                    catchId = tokens[catch_begin].content;
                    catch_begin+=1;
                    if(tokens[catch_begin].type!=RParen_TOKEN)
                        parseError(ctx,"SyntaxError","Invalid Syntax");
                    catch_begin+=1;
                    catch_begin = find_token_consecutive(bgscope_token,catch_begin,end,tokens);
                    int e = catch_begin;
                    if(catch_begin==-1)
                    {
                        catch_begin = find_token_consecutive(newline_token,e+1,end,tokens);
                        catch_end = find_token(newline_token,catch_begin+1,end,tokens);
                        if(catch_end==-1)
                            parseError(ctx,"SyntaxError","Error expected code block or line after catch");
                    }
                    else
                        catch_end = match_token(catch_begin,end,R_CURLY_BRACKET_TOKEN,tokens);
                }
                else
                {
                    parseError(ctx,"SyntaxError","Error expected catch block after try{}!");
                }
                catch_begin += 1;
                catch_end -= 1;
                nodeptr_vector_push(&(ast->childs),new_node(ID_NODE,catchId));
                bool ctxCopy = ctx->intry;
                ctx->intry = true;
                nodeptr_vector_push(&(ast->childs),parse_block(ctx,tokens,try_begin,try_end));
                ctx->intry = ctxCopy;
                ctxCopy = ctx->incatch;
                ctx->incatch = true;
                nodeptr_vector_push(&(ast->childs),parse_block(ctx,tokens,catch_begin,catch_end));
                ctx->incatch = ctxCopy;
                
                begin=catch_end+2;
                k = catch_end+1;
            }
            else if(ast->type==BREAK_node || ast->type==CONTINUE_node)
            {
                if(!ctx->inloop)
                    parseError(ctx,"SyntaxError","Error use of break or continue not allowed outside loop!");
                begin = k+1;
            }
            else if(ast->type==RETURN_NODE)
            {
                if(!ctx->infunc)
                    parseError(ctx,"SyntaxError","Error use of return statement outside functon!");
                begin = k+1;
            }
            else if(ast->type==import)
            {
                const char* str = ast->childs.arr[1]->val;
                if(ast->childs.arr[1]->type!=ID_NODE)
                {
                    Node* A;
                    if( str_vector_search(ctx->files,str) != -1)
                    {
                        A = new_node(file_node,"");
                        add_child(A,new_node(line_node,int64_to_string(ctx->line_num)));
                        add_child(A,new_node(STR_NODE,str));
                        add_child(A,new_node(EOP,""));
                    }
                    else
                    {
                        FILE* file = fopen(str,"rb"); // IMPORTANT: make sure to use readfile in future
                        if(!file)
                            parseError(ctx,"ImportError",strerror(errno));
                        str_vector_push(ctx->files,clone_str(str));

                        fseek(file,0,SEEK_END);
                        size_t total = ftell(file);
                        char* src = malloc(sizeof(char)*(total+1));
                        rewind(file);
                        if(fread(src,sizeof(char),total,file) != total)
                            parseError(ctx,"ImportError","Error opening file.");
                        src[total] = 0;
                        fclose(file);
                        str_vector_push(ctx->sources,src);
                        const char* F = ctx->filename;
                        size_t K = ctx->line_num;
                        ctx->filename = str;
                        ctx->line_num = 1;
                        lexer lex;
                        zuko_src tmp;
                        zuko_src_init(&tmp);
                        zuko_src_add_file(&tmp, clone_str(ctx->filename),src);
                        //tmp.addFile(filename,src);
                        token_vector t = lexer_generateTokens(&lex,&tmp,true,0);
                        if(lex.hadErr)
                        {
                            ctx->filename = F;
                            ctx->line_num = K;
                            //error was already printed
                            exit(1);
                        }
                        A = new_node(file_node,"");
                        add_child(A,new_node(line_node,int64_to_string(K)));
                        add_child(A,new_node(STR_NODE,str));
                        Node* subast;
                            subast = parse_block(ctx,t.arr,0,t.size-1);
                        add_child(A,subast);
                        ctx->filename = F;
                        ctx->line_num = K;

                    }
                    begin = k+1;
                    delete_ast(ast);
                    ast = A;
                }
                else
                {
                    begin = k+1;
                }
            }
            else
                begin = k+1;

            if(!final)
            {
                final = ast;
                e = final;
            }
            else
            {
                add_child(e,ast);
                e = e->childs.arr[e->childs.size-1];
            }
        }
        k+=1;
    }
    add_child(e,new_node(EOP,""));
    free(multiline.arr);
    return final;
}

void parser_destroy(Parser* ctx)
{
    str_vector_destroy(&ctx->prefixes);
    token_vector_destroy(&ctx->known_constants);
}
