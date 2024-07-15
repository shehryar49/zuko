#include "parser.h"
#include "ast.h"
#include "lexer.h"
#include <string.h>
#include "convo.h"
#include "refgraph.h"
#include "str-vec.h"
#include "token.h"
#include "zuko-src.h"

extern bool REPL_MODE;
void REPL();


Node* NewNode(NodeType type,string val)
{
  Node* p = new(nothrow) Node;
  if(!p)
  {
    printf("Parser: Error allocating memory for AST!\n");
    exit(1);
  }
  p->val = val;
  p->type = type;
  return p;
}

void strip_newlines(const vector<token>& tokens,int& begin,int& end)
{
    while(end >= begin && tokens[begin].type==NEWLINE_TOKEN )
        begin+=1; 
    while(end >= begin && tokens[end].type==NEWLINE_TOKEN )
        end-=1;
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
int match_token(int k,TokenType end,const vector<token>& tokens)
{
    TokenType start = tokens[k].type;
    int ignore = 0;
    int l = tokens.size();
    while(k<l)
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
int match_token_right(int k,TokenType end,const vector<token>& tokens)
{
    TokenType start = tokens[k].type;
    
    int ignore = 0;
    while(k>=0)
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

int find_token(token t,int start,int end,const vector<token>& tokens)
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
  for(size_t k = 0;k<ast->childs.size();k+=1)
  {
     delete_ast(ast->childs[k]);
  }
  delete ast;
}
void copy_ast(Node*& dest,Node* src)
{
    //Copies one tree structure value by value( deep copy )
    if(!src)
        return;
    dest->val = src->val;
    int k = 0;
    int l = src->childs.size();
    for(k=0;k<l;k+=1)
    {
        Node* n = new Node;
        copy_ast(n,src->childs[k]);
        dest->childs.push_back(n);
    }
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
   printf("|%s %s\n",NodeTypeNames[(int)n->type],n->val.c_str());
   spaces+=2;
   for(size_t k=0;k<(n->childs.size());k+=1)
   {
        for(int j=0;j<spaces;j++)
           fputc(' ',stdout);
        print_ast(n->childs[k],spaces);
   }
}
int find_token_consecutive(token t,int start,int end,const vector<token>& tokens)
{
    for(int k=start;k<=end;k+=1)
    {
        if(tokens[k].type==t.type && strcmp(tokens[k].content,t.content) == 0)
            return k;
        else if(tokens[k].type== TokenType::NEWLINE_TOKEN)
            ;
        else
            return -1;
    }
    return -1;
}


void Parser::set_source(zuko_src* p,size_t root_idx)
{
    if(root_idx >= p->files.size)
    {
        fprintf(stderr,"Parser.init() failed. Invalid root_idx!");
        return;
    }
    num_of_constants = &p->num_of_constants;
    refGraph = &p->ref_graph;
    files = &p->files;
    sources = &p->sources;
    filename = p->files.arr[root_idx];
    currSym = ".main";
    inclass = false;
    infunc = false;
    inloop = false;
    inif = false;
    inelif = false;
    inelse = false;
    intry = false;
    incatch = false;
    foundYield = false;
    str_vector v;
    str_vector_init(&v);
    refgraph_emplace(refGraph,clone_str(".main"),v);
}
void Parser::parseError(string type,string msg)
{
    fprintf(stderr,"\nFile %s\n",filename.c_str());
    fprintf(stderr,"%s at line %zu\n",type.c_str(),line_num);

    int idx = str_vector_search(files,filename.c_str());
    const char* source_code = sources->arr[idx];
    size_t l = 1;
    string line = "";
    size_t k = 0;
    while(source_code[k]!=0 && l<=line_num)
    {
        if(source_code[k]=='\n')
            l+=1;
        else if(l==line_num)
            line+=source_code[k];
        k+=1;
    }
    fprintf(stderr,"%s\n",lstrip(line).c_str());
    fprintf(stderr,"%s\n",msg.c_str());
    if(REPL_MODE)
        REPL();
    exit(1);
}
char* clone_str(const char* str)
{
    size_t len = strlen(str);
    char* d = (char*)malloc(sizeof(char)*(len+1));
    strcpy(d,str);
    return d;
}
bool Parser::addSymRef(string name)
{
    /* adds edge from currSym to name, indicating curr symbol uses name*/
    if(name == currSym) // no self loops
        return false;
    if(!refgraph_getref(refGraph,name.c_str()))
        return false;
    str_vector* neighbours = refgraph_getref(refGraph,currSym.c_str());
    if(str_vector_search(neighbours,name.c_str()) == -1)
        str_vector_push(neighbours,clone_str(name.c_str()));
    return true;
}
Node* Parser::parseExpr(const vector<token>& tokens)
{

    if(tokens.size()==0)
        parseError("SyntaxError","Invalid Syntax");

    Node* ast = nullptr;

    if(tokens.size()==1)
    {

        if(tokens[0].type== TokenType::KEYWORD_TOKEN && strcmp(tokens[0].content,"nil")==0 )
        {
            ast = NewNode(NodeType::NIL,tokens[0].content);
            return ast;
        }
        else if(tokens[0].type== TokenType::BOOL_TOKEN )
        {
            ast = NewNode(NodeType::BOOL,tokens[0].content);
            return ast;
        }
        else if(tokens[0].type == TokenType::ID_TOKEN)
        {
            auto it = prefixes.rbegin();
            bool done = false;
            while(it != prefixes.rend())
            {
                aux = merge_str((*it),tokens[0].content);
                if((done = addSymRef(aux)))
                    break;
                it++;
            }
            ast = NewNode(NodeType::ID,tokens[0].content);
            return ast;
        }
        else if(tokens[0].type == TokenType::NUM_TOKEN && isnum(tokens[0].content))
        {
            ast = NewNode(NodeType::NUM,tokens[0].content);//int32
            return ast;
        }
        else if(tokens[0].type == TokenType::BYTE_TOKEN)
        {
            ast = NewNode(NodeType::BYTE,tokens[0].content);
            return ast;
        }
        else if(tokens[0].type == STRING_TOKEN)
        {
            ast = NewNode(NodeType::STR,tokens[0].content);
            return ast;
        }
        else if(tokens[0].type== TokenType::FLOAT_TOKEN || tokens[0].type== TokenType::NUM_TOKEN)
        {
            if(find_token(tokens[0],0,(int)known_constants.size()-1,known_constants)==-1)
            {
                known_constants.push_back(tokens[0]);
                *num_of_constants = *num_of_constants + 1;
            }
            if(tokens[0].type == FLOAT_TOKEN)
                ast = NewNode(NodeType::FLOAT,tokens[0].content);
            else if(tokens[0].type == NUM_TOKEN)//int64
                ast = NewNode(NodeType::NUM,tokens[0].content);
            return ast;
        }
        parseError("SyntaxError","Invalid Syntax");
    }


    /////////
    //Following precdence is in reverse order
    static vector<vector<string>> prec = {{"and","or","is"},{"<",">","<=",">=","==","!="},{"<<",">>","&","|","^"},{"+","-"},{"/","*","%"}};
    static vector<vector<NodeType>> opNodeTypes = 
            {
            {NodeType::AND,NodeType::OR,NodeType::IS},
            {NodeType::lt,NodeType::gt,NodeType::lte,NodeType::gte,NodeType::equal,NodeType::noteq},
            {NodeType::lshift,NodeType::rshift,NodeType::bitwiseand,NodeType::bitwiseor,NodeType::XOR},
            {NodeType::add,NodeType::sub},
            {NodeType::div,NodeType::mul,NodeType::mod}
            };
    static int l = prec.size();
    int k = tokens.size()-1;
    
    for(int i=0;i<l;++i)
    {
        vector<string>& op_set = prec[i];
        k = tokens.size()-1;
        std::vector<string>::iterator m = op_set.end();
        while(k>=0)
        {

        if(tokens[k].type== TokenType::RParen_TOKEN)
        {
            int i = match_token_right(k,TokenType::LParen_TOKEN,tokens);
            if(i==-1)
                parseError("SyntaxError","Invalid Syntax");
            k = i;
        }
        else if(tokens[k].type== TokenType::END_LIST_TOKEN)
        {
            int i = match_token_right(k,TokenType::BEGIN_LIST_TOKEN,tokens);
            if(i==-1)
                parseError("SyntaxError","Invalid Syntax");
            k=i;
        }
        else if(tokens[k].type== TokenType::R_CURLY_BRACKET_TOKEN)
        {
            int i = match_token_right(k,TokenType::L_CURLY_BRACKET_TOKEN,tokens);
            if(i==-1)
                parseError("SyntaxError","Invalid Syntax");
            k=i;
        }
        else if(tokens[k].type== TokenType::OP_TOKEN && ((m = std::find(op_set.begin(),op_set.end(),tokens[k].content))!=op_set.end()) && k!=0)
        {
            vector<token> lhs = {tokens.begin(),tokens.begin()+k};
            vector<token> rhs = {tokens.begin()+k+1,tokens.end()};
            if(lhs.size()==0 || rhs.size()==0)
                parseError("SyntaxError","Invalid Syntax");
            int l = m - op_set.begin();
            Node* ast = NewNode(opNodeTypes[i][l]);
            ast->childs.push_back(parseExpr(lhs));
            ast->childs.push_back(parseExpr(rhs));
            return ast;
        }
        k-=1;
        }
    }
    //////

        /////////////////
    //Unary Operators
        ////////////////
    if(tokens[0].type== TokenType::OP_TOKEN)
    {

        if(strcmp(tokens[0].content,"-") == 0)
        {
            Node* ast = NewNode(NodeType::neg);
            vector<token> e = {tokens.begin()+1,tokens.end()};
            if(e.size()==0)
                parseError("SyntaxError","Invalid Syntax");
            size_t sz = known_constants.size();
            Node* E = parseExpr(e);
            if(known_constants.size()==sz+1 && e.size()==1 && (known_constants[sz].type==FLOAT_TOKEN || known_constants[sz].type==NUM_TOKEN))
            {
                known_constants[sz].content = clone_str(((string)"-"+(string)known_constants[sz].content).c_str());
                if(known_constants[sz].type == FLOAT_TOKEN)
                {
                    ast->type = NodeType::FLOAT;
                    ast->val = known_constants[sz].content;
                }
                else if(known_constants[sz].type == NUM_TOKEN)
                {
                    ast->type = NodeType::NUM;
                    ast->val = known_constants[sz].content;
                }
                return ast;
            }
            ast->childs.push_back(E);
            return ast;
        }
        else if(strcmp(tokens[0].content,"+") == 0)
        {
            vector<token> e = {tokens.begin()+1,tokens.end()};
            if(e.size()==0)
                parseError("SyntaxError","Invalid Syntax");
            return (parseExpr(e));
        }
        else if(strcmp(tokens[0].content,"~") == 0)
        {
            Node* ast = NewNode(NodeType::complement);
            vector<token> e = {tokens.begin()+1,tokens.end()};
            if(e.size()==0)
                parseError("SyntaxError","Invalid Syntax");
            ast->childs.push_back(parseExpr(e));
            return ast;
        }
        else if(strcmp(tokens[0].content,"!") == 0)
        {
            Node* ast = NewNode(NodeType::NOT);
            vector<token> e = {tokens.begin()+1,tokens.end()};
            ast->childs.push_back(parseExpr(e));
            return ast;
        }
    }
    if(tokens[tokens.size()-1].type==END_LIST_TOKEN)// evaluate indexes or lists
    {
        int i = match_token_right((int)tokens.size()-1,TokenType::BEGIN_LIST_TOKEN,tokens);
        if(i==-1)
            parseError("SyntaxError","Invalid Syntax");
        if(i!=0)
        {
            vector<token> toindex = {tokens.begin(),tokens.begin()+i};
            vector<token> index = {tokens.begin()+i+1,tokens.end()-1};
            Node* ast = NewNode(NodeType::index);
            ast->childs.push_back(parseExpr(toindex));
            ast->childs.push_back(parseExpr(index));
            return ast;
        }
        else
        {
            ast = NewNode(NodeType::list);
            if(tokens.size()==2)//empty list
                return ast;
            vector<token> t;
            int L = tokens.size()-2;//number of tokens excluding square brackets
            for(int k = 1;k<=L;k+=1)//process list elements
            {
            if(tokens[k].type== TokenType::COMMA_TOKEN)
            {
                if(t.size()==0)
                    parseError("SyntaxError","Error expected an element before ',' ");
                ast->childs.push_back(parseExpr(t));
                t.clear();
            }
            else if(tokens[k].type== TokenType::BEGIN_LIST_TOKEN)
            {
                int R = match_token(k,TokenType::END_LIST_TOKEN,tokens);
                if(R>(L) || R==-1)
                    parseError("SyntaxError","Invalid Syntax");
                vector<token> sublist = {tokens.begin()+k,tokens.begin()+R+1};
                t.insert(t.end(),sublist.begin(),sublist.end());
                k = R;
            }
            else if(tokens[k].type== TokenType::L_CURLY_BRACKET_TOKEN)
            {
                int R = match_token(k,TokenType::R_CURLY_BRACKET_TOKEN,tokens);
                if(R>L || R==-1)
                    parseError("SyntaxError","Invalid Syntax");
                vector<token> subdict = {tokens.begin()+k,tokens.begin()+R+1};
                t.insert(t.end(),subdict.begin(),subdict.end());
                k = R;
            }
            else if(tokens[k].type==TokenType::ID_TOKEN && tokens[k+1].type==TokenType::LParen_TOKEN)
            {
                int R = match_token(k,TokenType::RParen_TOKEN,tokens);
                if(R>L || R==-1)
                    parseError("SyntaxError","Invalid Syntax");
                vector<token> subfn = {tokens.begin()+k,tokens.begin()+R+1};
                t.insert(t.end(),subfn.begin(),subfn.end());
                k = R;
            }
            else
                t.push_back(tokens[k]);
            }
            if(t.size()==0)
                parseError("SyntaxError","Error expected an element after ',' ");
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
            int i = match_token_right(k,TokenType::LParen_TOKEN,tokens);
            if(i==-1)
                parseError("SyntaxError","Invalid Syntax");
            k = i;
        }
        else if(tokens[k].type== TokenType::END_LIST_TOKEN)
        {
            int i = match_token(k,TokenType::BEGIN_LIST_TOKEN,tokens);
            if(i==-1)
                parseError("SyntaxError","Invalid Syntax");
            k=i;
        }
        else if(tokens[k].type== TokenType::R_CURLY_BRACKET_TOKEN)
        {
            int i = match_token_right(k,TokenType::L_CURLY_BRACKET_TOKEN,tokens);
            if(i==-1)
                parseError("SyntaxError","Invalid Syntax");
            k = i;
        }
        else if(tokens[k].type== TokenType::OP_TOKEN && (strcmp(tokens[k].content,".")==0))
        {
            vector<token> lhs = {tokens.begin(),tokens.begin()+k};
            vector<token> rhs = {tokens.begin()+k+1,tokens.end()};
            if(lhs.size()==0 || rhs.size()==0)
                parseError("SyntaxError","Invalid Syntax");
            Node* ast = NewNode(NodeType::memb);
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
                ast = NewNode(NodeType::dict);
                if(tokens.size()==2)
                    return ast;
                vector<token> Key;
                vector<token> Value;
                string tofill = "key";
                int L = tokens.size()-2;
                for(int k = 1;k<=L;k+=1)
                {
                if(tokens[k].type== TokenType::COMMA_TOKEN)
                {
                    if(Value.size()==0)
                        parseError("SyntaxError","Invalid Syntax");
                    ast->childs.push_back(parseExpr(Value));
                    Value.clear();
                    tofill = "key";
                }
                else if(tokens[k].type== TokenType::COLON_TOKEN)
                {
                    if(Key.size()==0)
                        parseError("SyntaxError","Invalid Syntax");
                    ast->childs.push_back(parseExpr(Key));
                    Key.clear();
                    tofill = "value";
                }
                else if(tokens[k].type== TokenType::BEGIN_LIST_TOKEN)
                {
                    int R = match_token(k,TokenType::END_LIST_TOKEN,tokens);
                    vector<token> sublist ={tokens.begin()+k,tokens.begin()+R+1};
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
                    int R = match_token(k,TokenType::R_CURLY_BRACKET_TOKEN,tokens);
                    if(R==(int)tokens.size()-1)
                        parseError("SyntaxError","Invalid Syntax");

                    vector<token> subdict ={tokens.begin()+k,tokens.begin()+R+1};
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
                }
                else if(tokens[k].type==TokenType::ID_TOKEN && tokens[k+1].type==TokenType::LParen_TOKEN)
                {
                    int R = match_token(k,TokenType::RParen_TOKEN,tokens);
                    if(R>((int)tokens.size()-2) || R==-1)
                        parseError("SyntaxError","Invalid Syntax");
                    vector<token> subfn = {tokens.begin()+k,tokens.begin()+R+1};
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
                    parseError("SyntaxError","Invalid Syntax");
                ast->childs.push_back(parseExpr(Value));
                Value.clear();
                return ast;
            
        }
    }
    ////////////////////
    ////////////////////
    if(tokens[0].type== TokenType::ID_TOKEN && tokens[1].type== TokenType::LParen_TOKEN)
    {
        int i = match_token(1,TokenType::RParen_TOKEN,tokens);
        if(i==-1)
            parseError("SyntaxError","Invalid Syntax");
        if(i==(int)tokens.size()-1)
        {
            Node* ast = NewNode(NodeType::call);
            Node* args = NewNode(NodeType::args);

            Node* n = NewNode(NodeType::line,to_string(tokens[0].ln));
            ast->childs.push_back(n);
            ast->childs.push_back(NewNode(NodeType::ID,tokens[0].content));
            auto it = prefixes.rbegin();
            bool done = false;
            while(it != prefixes.rend())
            {
                aux = merge_str((*it),tokens[0].content);
                if((done = addSymRef(aux)))
                    break;
                it++;
            }
            vector<token> T;
            vector<token> Args = {tokens.begin()+2,tokens.end()-1};
            if(Args.size()==0)
            {
                ast->childs.push_back(args);
                return ast;
            }
            for(int k = 0;k<(int)Args.size();k+=1)
            {
                if(Args[k].type== TokenType::COMMA_TOKEN)
                {
                    if(T.size()==0)
                        parseError("SyntaxError","Invalid Syntax");
                    args->childs.push_back(parseExpr(T));
                    T.clear();
                }
                else if(Args[k].type== TokenType::BEGIN_LIST_TOKEN)
                {
                    int i = match_token(k,TokenType::END_LIST_TOKEN,Args);
                    if(i==-1)
                        parseError("SyntaxError","Invalid Syntax");
                    vector<token> LIST =  {Args.begin()+k,Args.begin()+i+1};
                    T.insert(T.end(),LIST.begin(),LIST.end());
                    k = i;
                }
                else if(Args[k].type== TokenType::LParen_TOKEN)
                {
                    int i = match_token(k,TokenType::RParen_TOKEN,Args);
                    if(i==-1)
                        parseError("SyntaxError","Invalid Syntax");
                    vector<token> P =  {Args.begin()+k,Args.begin()+i+1};
                    T.insert(T.end(),P.begin(),P.end());
                    k = i;
                }
                else if(Args[k].type== TokenType::L_CURLY_BRACKET_TOKEN)
                {
                    int i = match_token(k,TokenType::R_CURLY_BRACKET_TOKEN,Args);
                    if(i==-1)
                        parseError("SyntaxError","Invalid Syntax");
                    vector<token> P =  {Args.begin()+k,Args.begin()+i+1};
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
        }//
    }
    ///////////////////
    //////////////
    if(tokens[0].type== TokenType::LParen_TOKEN)
    {
        int i = match_token(0,TokenType::RParen_TOKEN,tokens);
        if(i==-1)
            parseError("SyntaxError","Invalid Syntax");
        if(i==(int)tokens.size()-1)
        {
            vector<token> e = {tokens.begin()+1,tokens.end()-1};
            return parseExpr(e);
        }
    }
    string expr;
    bool multiline=false;
    for(auto e: tokens)
    {
        if(e.type==NEWLINE_TOKEN)
        {
            multiline=true;
            break;
        }
        if(e.type==TokenType::STRING_TOKEN)
            expr+= "\"" + (string)e.content + "\"";
        else
            expr+=e.content;
    }
    if(!multiline)
        parseError("SyntaxError","Invalid syntax: "+expr);
    else
        parseError("SyntaxError",expr);  
    return ast;
}

Node* Parser::parseStmt(const vector<token>& tokens,size_t begin,size_t end)
{
    size_t tokens_size = end-begin+1;
    if(tokens_size==0)
        parseError("SyntaxError","Invalid Syntax");

    if(tokens_size==1 && tokens[begin].type==KEYWORD_TOKEN && strcmp(tokens[begin].content,"gc") == 0)
        return NewNode(NodeType::gc);

    bool isPrivate = false;
    if(tokens[begin].type==TokenType::KEYWORD_TOKEN && strcmp(tokens[begin].content,"private") == 0)
    {
        isPrivate = true;
        begin+=1;
        if(!inclass)
            parseError("SyntaxError","Error use of keyword private outside class!");

        if(tokens[begin].type==TokenType::KEYWORD_TOKEN && strcmp(tokens[begin].content,"var") == 0);
        else if(tokens[begin].type==TokenType::KEYWORD_TOKEN && strcmp(tokens[begin].content,"function") == 0);
        else
            parseError("SyntaxError","Invalid use of keyword private\n");
    }
    else if(tokens[begin].type==TokenType::KEYWORD_TOKEN && strcmp(tokens[begin].content,"public") == 0)
    {
        isPrivate = false;
        //tokens.erase(tokens.begin());
        begin += 1;
        if(!inclass)
            parseError("SyntaxError","Error use of keyword public outside class!");
        if(tokens[begin].type==TokenType::KEYWORD_TOKEN && strcmp(tokens[begin].content,"var") == 0);
        else if(tokens[begin].type==TokenType::KEYWORD_TOKEN && strcmp(tokens[begin].content,"function") == 0);
        else
            parseError("SyntaxError","Invalid use of keyword public");
    }

    if(tokens[begin].type== TokenType::KEYWORD_TOKEN  && strcmp(tokens[begin].content,"var") == 0)
    {
        //declare statement
        if(tokens_size < 4) // there must be minimum four tokens var x  (simplest case)        
            parseError("SyntaxError","Invalid declare statement!");

        if(tokens[begin+1].type!= TokenType::ID_TOKEN || (tokens[begin+2].type!= TokenType::OP_TOKEN &&  strcmp(tokens[begin+2].content,"=")!=0))
            parseError("SyntaxError","invalid declare statement!");
    
        Node* ast = NewNode(NodeType::declare);
        Node* n = NewNode(NodeType::line,to_string(tokens[begin].ln));
        ast->childs.push_back(n);
        if(((string)tokens[1].content).find("::")!=string::npos)
            parseError("SyntaxError","Invalid Syntax");
        const char* fnprefix = prefixes.back();
        const char* tmp = tokens[begin+1].content;
        if(atGlobalLevel())
            tmp = merge_str(fnprefix,tokens[begin+1].content);
        if(isPrivate)
            tmp = merge_str("@",tokens[begin+1].content);
        ast->val = tmp;
        vector<token> expr = {tokens.begin()+begin+3,tokens.begin()+end+1};
        if(expr[0].type==KEYWORD_TOKEN && strcmp(expr[0].content , "yield") == 0)
        {
            foundYield = true;
            expr = {expr.begin()+1,expr.end()};
            ast->childs.push_back(NewNode(NodeType::YIELD));
            if(expr.size()==0)
                parseError("SyntaxError","Error expected expression after return keyword");
            Node* n = NewNode(NodeType::line ,to_string(tokens[begin].ln));
            ast->childs.back()->childs.push_back(n);
            ast->childs.back()->childs.push_back(parseExpr(expr));
            return ast;
        }
        ast->childs.push_back(parseExpr(expr));
        return ast;
    }

    if(tokens[begin].type== TokenType::KEYWORD_TOKEN && (strcmp(tokens[begin].content,"break")==0 || strcmp(tokens[begin].content,"continue") == 0) && begin==end)
    {
        Node* ast = NewNode((strcmp(tokens[begin].content , "break") == 0) ? NodeType::BREAK : NodeType::CONTINUE);
        ast->childs.push_back(NewNode(NodeType::line,to_string(tokens[begin].ln)));
        return ast;
    }
    if(tokens[begin].type== TokenType::ID_TOKEN)
    {
        if(tokens_size>=3)
        {
            if(tokens[begin+1].type== TokenType::LParen_TOKEN && tokens[end].type== TokenType::RParen_TOKEN && match_token(begin+1,TokenType::RParen_TOKEN,tokens)==(int)end)
            {
                bool wrapInPrint = false;
                if(REPL_MODE && atGlobalLevel() && strcmp(tokens[begin].content,"print")!=0 && strcmp(tokens[begin].content,"println")!=0 && strcmp(tokens[begin].content,"printf")!=0)
                    wrapInPrint = true;
                Node* ast = NewNode(NodeType::call);
                Node* n = NewNode(NodeType::line,to_string(tokens[begin].ln));
                ast->childs.push_back(n);
                ast->childs.push_back(NewNode(NodeType::ID,tokens[begin].content));
                auto it = prefixes.rbegin();
                bool done = false;
                while(it != prefixes.rend())
                {
                    aux = merge_str((*it),tokens[begin].content);
                    if((done = addSymRef(aux)))
                        break;
                    it++;
                }
                Node* args = NewNode(NodeType::args);
                if(tokens_size==3)
                {
                    ast->childs.push_back(args);  
                }
                else
                {
                    vector<token> T;
                    vector<token> Args = {tokens.begin()+begin+2,tokens.begin()+end};
                    for(int k = 0;k<(int)Args.size();k+=1)
                    {
                        if(Args[k].type== TokenType::COMMA_TOKEN)
                        {
                            if(T.size()==0)
                                parseError("SyntaxError","Invalid Syntax");
                            args->childs.push_back(parseExpr(T));
                            T.clear();
                        }
                        else if(Args[k].type== TokenType::BEGIN_LIST_TOKEN)
                        {
                            int i = match_token(k,TokenType::END_LIST_TOKEN,Args);
                            if(i==-1)
                                parseError("SyntaxError","Invalid Syntax");
                            vector<token> P = {Args.begin()+k,Args.begin()+i+1};
                            T.insert(T.end(),P.begin(),P.end());
                            k = i;
                        }
                        else if(Args[k].type== TokenType::L_CURLY_BRACKET_TOKEN)
                        {
                            int i = match_token(k,TokenType::R_CURLY_BRACKET_TOKEN,Args);
                            if(i==-1)
                                parseError("SyntaxError","Invalid Syntax");
                            vector<token> P = {Args.begin()+k,Args.begin()+i+1};
                            T.insert(T.end(),P.begin(),P.end());
                            k = i;
                        }
                        else if(Args[k].type== TokenType::LParen_TOKEN)
                        {
                            int i = match_token(k,TokenType::RParen_TOKEN,Args);
                            if(i==-1)
                                parseError("SyntaxError","Invalid Syntax");
                            vector<token> P = {Args.begin()+k,Args.begin()+i+1};
                            T.insert(T.end(),P.begin(),P.end());
                            k = i;
                        }
                        else
                            T.push_back(Args[k]);
                }
                    if(T.size()==0)
                        parseError("SyntaxError","Invalid Syntax");
                    args->childs.push_back(parseExpr(T));
                    T.clear();
                    ast->childs.push_back(args);
                }
                if(wrapInPrint)
                {
                    Node* p = NewNode(NodeType::call);
                    Node* n = NewNode(NodeType::line,to_string(tokens[0].ln));
                    p->childs.push_back(n);
                    p->childs.push_back(NewNode(NodeType::ID,"println"));
                    Node* args = NewNode(NodeType::args);
                    args->childs.push_back(ast);
                    p->childs.push_back(args);
                    return p;
                }
                return ast;
            }
        }
    }

    if(tokens[begin].type==TokenType::KEYWORD_TOKEN && strcmp(tokens[begin].content,"class") == 0)
    {
        if(tokens_size<2 || tokens[begin+1].type != ID_TOKEN)
            parseError("SyntaxError","Invalid Syntax");
        bool extendedClass=false;
        if(tokens_size >=4)
        {
            if(tokens[begin+2].type!=TokenType::KEYWORD_TOKEN || tokens[begin+3].type!=TokenType::ID_TOKEN)
                parseError("SyntaxError","Invalid Syntax");
            if(strcmp(tokens[begin+2].content,"extends")!=0)
                parseError("SyntaxError","Invalid Syntax");
            extendedClass = true;
        }
        else if(tokens_size==2)
        {

        }
        else
            parseError("SyntaxError","Invalid Syntax");
        Node* ast;
        if(!extendedClass)
            ast = NewNode(NodeType::CLASS);
        else
            ast = NewNode(NodeType::EXTCLASS);
        Node* n = NewNode(NodeType::line,to_string(line_num));
        ast->childs.push_back(n);
        //Do not allow class names having '::'
        if(((string)tokens[1].content).find("::")!=string::npos)
            parseError("SyntaxError","Invalid Syntax");
        const char* fnprefix = prefixes.back();
        const char* tmp = tokens[begin+1].content;
        if(atGlobalLevel())
            tmp = merge_str(fnprefix,tokens[begin+1].content);
        ast->childs.push_back(NewNode(NodeType::ID,tmp));
        if(extendedClass)
        {
            vector<token> expr = {tokens.begin()+begin+3,tokens.begin()+end+1};
            ast->childs.push_back(parseExpr(expr));
        }
        return ast;
    }
    if(tokens[begin].type== TokenType::KEYWORD_TOKEN && (strcmp(tokens[begin].content,"while")==0 || strcmp(tokens[begin].content,"dowhile") == 0))
    {
        if(tokens_size>= 4 && tokens[begin+1].type== TokenType::LParen_TOKEN  && tokens[end].type== TokenType::RParen_TOKEN)
        {
            vector<token> expr = {tokens.begin()+begin+2,tokens.begin()+end};
            Node* ast = NewNode((strcmp(tokens[begin].content , "while") == 0) ? NodeType::WHILE : NodeType::DOWHILE);
            Node* n = NewNode(NodeType::line ,to_string(tokens[begin].ln));
            ast->childs.push_back(n);
            ast->childs.push_back(parseExpr(expr));
            return ast;
        }   
    }
    if(tokens[begin].type==TokenType::KEYWORD_TOKEN && strcmp(tokens[begin].content,"for") == 0)
    {
        if(tokens_size<3)
            parseError("SyntaxError","Invalid Syntax");
        if(tokens[begin+1].type!=TokenType::LParen_TOKEN || tokens[end].type!=TokenType::RParen_TOKEN)
        {
            parseError("SyntaxError","Invalid Syntax");
        }
        begin += 2;
        end -= 1;
        tokens_size = end - begin+1;

        token t;
        t.type = TokenType::KEYWORD_TOKEN;
        t.content = "to";
        int i = find_token(t,begin,end,tokens);
        bool dtoLoop = false;
        if(i==-1 || i > end)
        {
            t.content = "dto";
            i = find_token(t,begin,end,tokens);
            if(i == -1 || i > end)
                parseError("SyntaxError","Invalid Syntax");
            dtoLoop = true;
        }
        vector<token> init = {tokens.begin()+begin,tokens.begin()+i};

        int p = i;
        t.content = "step";
        t.type = TokenType::KEYWORD_TOKEN;
        i = find_token(t,i+1,end,tokens);
        vector<token> endpoint;
        vector<token> inc;
        if(i==-1)//step is optional
        {
            endpoint = {tokens.begin()+p+1,tokens.begin()+end+1};
            token one;
            one.type = TokenType::NUM_TOKEN;
            if(dtoLoop)
                one.content = "1";
            else
                one.content = "1";
            inc = {one};
        }
        else
        {
            endpoint = {tokens.begin()+p+1,tokens.begin()+i};
            inc = {tokens.begin()+i+1,tokens.begin()+end+1};
        }
        bool decl = false;
        if(init[0].type==TokenType::KEYWORD_TOKEN && strcmp(init[0].content,"var")==0 )
        {
            init.erase(init.begin());
            decl = true;
        }
        if(init[0].type==TokenType::ID_TOKEN && init[1].type==TokenType::OP_TOKEN && strcmp(init[1].content,"=") == 0)
        {
            vector<token> initExpr = {init.begin()+2,init.end()};
            Node* ast;
            if(dtoLoop)
                ast = NewNode(NodeType::DFOR);
            else
                ast = NewNode(NodeType::FOR);
            Node* n = NewNode(NodeType::line,to_string(tokens[begin].ln));
            ast->childs.push_back(n);
            if(decl)
                ast->childs.push_back(NewNode(NodeType::decl));
            else
                ast->childs.push_back(NewNode(NodeType::nodecl));
            ast->childs.push_back(NewNode(NodeType::ID,init[0].content));
            ast->childs.push_back(parseExpr(initExpr));
            ast->childs.push_back(parseExpr(endpoint));
            ast->childs.push_back(parseExpr(inc));
            return ast;
        }
        else
            parseError("SyntaxError","Invalid Syntax");
    }
    if(tokens[begin].type==TokenType::KEYWORD_TOKEN && strcmp(tokens[begin].content,"foreach") == 0)
    {
        if(tokens_size<3)
            parseError("SyntaxError","Invalid Syntax");

        if(tokens[begin+1].type!=TokenType::LParen_TOKEN || tokens[end].type!=TokenType::RParen_TOKEN)
            parseError("SyntaxError","Invalid Syntax");
        //tokens = {tokens.begin()+2,tokens.end()-1};
        begin += 2;
        end -= 1;
        tokens_size = end - begin+1;
        if(tokens_size<4)
            parseError("SyntaxError","Invalid Syntax");
        if(tokens[begin].type!=TokenType::KEYWORD_TOKEN || strcmp(tokens[begin].content,"var")!=0 || tokens[begin+1].type!=TokenType::ID_TOKEN || tokens[begin+2].type!=TokenType::COLON_TOKEN)
            parseError("SyntaxError","Invalid Syntax");
        vector<token> expr = {tokens.begin()+begin+3,tokens.begin()+end+1};
      

        Node* exprAST = parseExpr(expr);
        Node* ast = NewNode(NodeType::FOREACH);
        Node* n = NewNode(NodeType::line,to_string(tokens[begin].ln));
        ast->childs = {n,NewNode(NodeType::ID,tokens[begin+1].content),exprAST};
        token Q;
        Q.type = TokenType::NUM_TOKEN;
        Q.content = "-1";
        vector<token> E = {Q};
        ast->childs.push_back(parseExpr(E));
        return ast;
    }
    if(tokens[begin].type==TokenType::KEYWORD_TOKEN && strcmp(tokens[begin].content,"namespace") == 0)
    {
        if(tokens_size!=2)
            parseError("SyntaxError","Invalid Syntax");
        if(tokens[begin+1].type!=ID_TOKEN)
            parseError("SyntaxError","Invalid Syntax");
        Node* ast = NewNode(NodeType::NAMESPACE);
        Node* n = NewNode(NodeType::line,to_string(tokens[begin].ln));
        ast->childs.push_back(n);
        string fnprefix;
        if(((string)tokens[1].content).find("::")!=string::npos)
            parseError("SyntaxError","Invalid Syntax");

        ast->childs.push_back(NewNode(NodeType::ID,tokens[begin+1].content));
        return ast;
    }
    if(tokens[begin].type== TokenType::KEYWORD_TOKEN && strcmp(tokens[begin].content,"if") == 0)
    {
        if(tokens_size>=4)
        {
            if(tokens[begin+1].type== TokenType::LParen_TOKEN  && tokens[end].type== TokenType::RParen_TOKEN)
            {
                vector<token> expr = {tokens.begin()+begin+2,tokens.begin()+end};
                Node* ast = NewNode(NodeType::IF);
                Node* n = NewNode(NodeType::line,to_string(tokens[begin].ln));
                ast->childs.push_back(n);
                Node* conditions = NewNode(NodeType::conditions);
                conditions->childs.push_back(parseExpr(expr));
                ast->childs.push_back(conditions);
                return ast;
            }
        }
    }
    if(tokens[begin].type== TokenType::KEYWORD_TOKEN && strcmp(tokens[begin].content,"import") == 0)
    {
        Node* ast = NewNode(NodeType::import);
        vector<token> t;
        if(tokens_size==2)
        {
            if(tokens[begin+1].type==ID_TOKEN)
            {
                t.push_back(tokens[begin+1]);
                Node* n = NewNode(NodeType::line,to_string(tokens[begin].ln));
                ast->childs.push_back(n);
                ast->childs.push_back(parseExpr(t));
                return ast;
            }
            if(tokens[begin+1].type== TokenType::STRING_TOKEN)
            {
                t.push_back(tokens[begin+1]);
                Node* n = NewNode(NodeType::line,to_string(tokens[begin].ln));
                ast->childs.push_back(n);
                ast->childs.push_back(NewNode(NodeType::STR,tokens[begin+1].content));
                return ast;
            }
            else
            {
                    //line_num = tokens[1].ln;
                    parseError("SyntaxError","Invalid Syntax");
            }
        }
        else if(tokens_size==4 && strcmp(tokens[begin+2].content , "as") == 0)
        {
            if(tokens[begin+1].type!=ID_TOKEN || tokens[begin+2].type!=KEYWORD_TOKEN || strcmp(tokens[begin+2].content,"as")!=0 || tokens[begin+3].type!=ID_TOKEN)
            {
                parseError("SyntaxError","Invalid Syntax");
            }
            ast->type = NodeType::importas;
            t.push_back(tokens[begin+1]);
            Node* n = NewNode(NodeType::line,to_string(tokens[begin].ln));
            ast->childs.push_back(n);
            ast->childs.push_back(parseExpr(t));
            ast->childs.push_back(NewNode(NodeType::ID,tokens[begin+3].content));
            return ast;
        }
        else if(tokens_size==4)
        {
            if(tokens[begin+1].type!=ID_TOKEN || strcmp(tokens[begin+1].content,"std")!=0 || tokens[begin+2].type!=OP_TOKEN || strcmp(tokens[begin+2].content,"/")!=0 || tokens[begin+3].type!=ID_TOKEN )
                parseError("SyntaxError","Invalid Syntax");
            char* tmp;
            #ifdef _WIN32
            tokens[begin+3].content="C:\\zuko\\std\\"+tokens[begin+3].content+".zk";
            #else
            tmp = merge_str(merge_str("/opt/zuko/std/",tokens[begin+3].content),".zk");
            #endif
            Node* n = NewNode(NodeType::line,to_string(tokens[begin].ln));
            ast->childs.push_back(n);
            ast->childs.push_back(NewNode(NodeType::STR,tmp));
            return ast;
        }
        else
        {
            parseError("SyntaxError","Invalid Syntax");
        }
    }
    if(tokens[begin].type== TokenType::KEYWORD_TOKEN && strcmp(tokens[begin].content,"return") == 0)
    {
        Node* ast = NewNode(NodeType::RETURN_NODE);
        vector<token> t = {tokens.begin()+begin+1,tokens.begin()+end+1};
        if(t.size()==0)
            parseError("SyntaxError","Error expected expression after return keyword");
        Node* n = NewNode(NodeType::line,to_string(tokens[begin].ln));
        ast->childs.push_back(n);
        ast->childs.push_back(parseExpr(t));
        return ast;
    }
    if(tokens[begin].type== TokenType::KEYWORD_TOKEN && strcmp(tokens[begin].content,"yield") == 0)
    {
        foundYield = true;
        Node* ast = NewNode(NodeType::YIELD);
        vector<token> t = {tokens.begin()+begin+1,tokens.begin()+end+1};
        if(t.size()==0)
            parseError("SyntaxError","Error expected expression after yield keyword");
        Node* n = NewNode(NodeType::line,to_string(tokens[0].ln));
        ast->childs.push_back(n);
        ast->childs.push_back(parseExpr(t));
        return ast;
    }
    if(tokens[begin].type== TokenType::KEYWORD_TOKEN && strcmp(tokens[begin].content,"function") == 0)
    {
        if(tokens_size<4)
            parseError("SyntaxError","Invalid Syntax");
        
        if(tokens[begin+1].type!=ID_TOKEN || tokens[end].type!=TokenType::RParen_TOKEN)
            parseError("SyntaxError","Invalid Syntax");
        if(((string)tokens[begin+1].content).find("::")!=string::npos)
            parseError("SyntaxError","Invalid Syntax");
        const char* fnprefix = prefixes.back();
        const char* tmp = tokens[begin+1].content;
        if(atGlobalLevel())
            tmp = merge_str(fnprefix,tokens[begin+1].content);
        if(isPrivate)
            tmp = merge_str("@" , tokens[begin+1].content);
        Node* ast = NewNode(NodeType::FUNC);
        Node* n = NewNode(NodeType::line,to_string(tokens[begin].ln));
        ast->childs.push_back(n);
        ast->childs.push_back(NewNode(NodeType::ID,tmp));
        ast->childs.push_back(NewNode(NodeType::args));
        if(tokens_size==4)
            return ast;
        vector<token> args = {tokens.begin()+begin+3,tokens.begin()+end};
        if(args.size()<2)
            parseError("SyntaxError","Invalid Syntax");
        size_t k = 0;
        bool found_default = false;//found any default argument
        string tname;
        while(k<args.size())
        {
            if((args[k].type== TokenType::KEYWORD_TOKEN && strcmp(args[k].content,"var") == 0))
            {
                if(k==args.size()-1)
                    parseError("SyntaxError","Invalid Syntax");
                if(args[k+1].type!= TokenType::ID_TOKEN)
                    parseError("SyntaxError","Invalid Syntax");
                ast->childs[2]->childs.push_back(NewNode(NodeType::ID,args[k+1].content));
                k+=1;
                if(k<args.size()-1)// or k+1 < args.size() or in simple English(there are more parameters)
                {
                    k+=1;//it must be a comma
                    if(args[k].type==OP_TOKEN && strcmp(args[k].content,"=")==0)//default parameters
                    {
                        k+=1;
                        if(k >= args.size())
                            parseError("SyntaxError","Invalid Syntax");
                        int j = k;
                        int i1 = 0;
                        int i2 = 0;
                        int i3 = 0;
                        bool found = false;
                        for(;j<(int)args.size();j++)
                        {
                        if(args[j].type==L_CURLY_BRACKET_TOKEN)
                            i1+=1;
                        else if(args[j].type==R_CURLY_BRACKET_TOKEN)
                            i1-=1;
                        else if(args[j].type==BEGIN_LIST_TOKEN)
                            i2+=1;
                        else if(args[j].type==END_LIST_TOKEN)
                            i2-=1;
                        else if(args[j].type==LParen_TOKEN)
                            i3+=1;
                        else if(args[j].type==RParen_TOKEN)
                            i3-=1;
                        else if(args[j].type==COMMA_TOKEN && i1==0 && i2==0 && i3==0)
                        {
                            found = true;
                            break;
                        }
                        }
                        if(!found && (size_t)j!=args.size())
                            parseError("SyntaxError","Invalid Syntax");
                        found_default = true;
                        vector<token> default_expr = {args.begin()+k,args.begin()+j};
                        ast->childs[2]->childs.back()->childs.push_back(parseExpr(default_expr));
                        k = j+1;
                        continue;
                    }
                    if(found_default)
                        parseError("SyntaxError","Invalid Syntax");
                    if(args[k].type!= TokenType::COMMA_TOKEN)
                        parseError("SyntaxError","Invalid Syntax");
                    if(k==args.size()-1)
                        parseError("SyntaxError","Invalid Syntax");
                    k+=1;
                    continue;
                }
            }
            else
                parseError("SyntaxError","Invalid Syntax");
            if(found_default)
                parseError("SyntaxError","Invalid Syntax");
            k+=1;
        }
        return ast;
    }
    int k=begin;
    while(k<=end)
    {
        if(tokens[k].type!= TokenType::OP_TOKEN)
        {
            k+=1;
            continue;
        }
        if(strcmp(tokens[k].content,"=")==0)
        {
            Node* ast = NewNode(NodeType::assign,tokens[k].content);
            vector<token> left = {tokens.begin()+begin,tokens.begin()+k};
            if(left.size()==0)
                parseError("SyntaxError","Invalid Syntax");
            Node* n = NewNode(NodeType::line,to_string(tokens[k].ln));
            ast->childs.push_back(n);
            ast->childs.push_back(parseExpr(left));
            vector<token> right = {tokens.begin()+k+1,tokens.begin()+end+1};
            if(right.size()==0)
                parseError("SyntaxError","Invalid Syntax");
            if(right[0].type==KEYWORD_TOKEN && strcmp(right[0].content,"yield")==0)
            {
                right = {right.begin()+1,right.end()};
                ast->childs.push_back(NewNode(NodeType::YIELD));
                if(right.size()==0)
                    parseError("SyntaxError","Error expected expression after return keyword");
                Node* n = NewNode(NodeType::line,to_string(tokens[k+1].ln));
                ast->childs.back()->childs.push_back(n);

                ast->childs.back()->childs.push_back(parseExpr(right));
                foundYield = true;
                return ast;
            }
            ast->childs.push_back(parseExpr(right));
            return ast;
        }
        else if(strcmp(tokens[k].content,"+=")==0 || strcmp(tokens[k].content,"-=")==0 || strcmp(tokens[k].content,"/=")==0 || strcmp(tokens[k].content,"*=")==0 || strcmp(tokens[k].content,"^=")==0 || strcmp(tokens[k].content,"%=")==0 || strcmp(tokens[k].content,"|=")==0 || strcmp(tokens[k].content,"&=")==0 || strcmp(tokens[k].content,"<<=")==0 || strcmp(tokens[k].content,">>=")==0)
        {
            Node* ast = NewNode(NodeType::assign);
            vector<token> left = {tokens.begin()+begin,tokens.begin()+k};
            //line_num  = tokens[k].ln;
            if(left.size()==0 )
                parseError("SyntaxError","Invalid Syntax");

            Node* n = NewNode(NodeType::line,to_string(tokens[k].ln));
            ast->childs.push_back(n);
            ast->childs.push_back(parseExpr(left));
            token o;
            o.type = OP_TOKEN;
            char* char_to_str(char);
            o.content= char_to_str(tokens[k].content[0]);
            left.push_back(o);
            vector<token> right = {tokens.begin()+k+1,tokens.begin()+end+1};
            if(right.size()==0)
                parseError("SyntaxError","Invalid Syntax");
            
            right.insert(right.begin(),left.begin(),left.end());
            ast->childs.push_back(parseExpr(right));
            return ast;
        }
        k+=1;
    }
    
    
    if(tokens[begin].type==TokenType::KEYWORD_TOKEN && strcmp(tokens[begin].content,"try") == 0)
    {
        Node* line = NewNode(NodeType::line,to_string(tokens[begin].ln));
        Node* ast = NewNode(NodeType::TRYCATCH);
        ast->childs.push_back(line);
        return ast;
    }
    if(tokens[begin].type==TokenType::KEYWORD_TOKEN && strcmp(tokens[begin].content,"throw") == 0)
    {
        if(tokens_size < 2)
            parseError("SyntaxError","Invalid Syntax");
        vector<token> expr = {tokens.begin()+begin+1,tokens.begin()+end+1};
        Node* line = NewNode(NodeType::line ,to_string(tokens[begin].ln));
        Node* ast = NewNode(NodeType::THROW);
        ast->childs.push_back(line);
        ast->childs.push_back(parseExpr(expr));
        return ast;
    }
    //Handle statements of the form
    //expr.fun()
    k = end;
    size_t iterator = begin; 
    while(iterator<=end)
    {
        if(tokens[k].type==TokenType::RParen_TOKEN)
        {
            int i = match_token_right(k,TokenType::LParen_TOKEN,tokens);
            if(i==-1)
                parseError("SyntaxError","Invalid Syntax");
            k = i;
        }
        else if(tokens[k].type== TokenType::END_LIST_TOKEN)
        {
            int i = match_token_right(k,TokenType::BEGIN_LIST_TOKEN,tokens);
            if(i==-1)
                parseError("SyntaxError","Invalid Syntax");
            k=i;
        }
        else if(tokens[k].type== TokenType::R_CURLY_BRACKET_TOKEN)
        {
            int i = match_token_right(k,TokenType::L_CURLY_BRACKET_TOKEN,tokens);
            if(i==-1)
                parseError("SyntaxError","Invalid Syntax");
            k=i;
        }
        else if(tokens[k].type== TokenType::OP_TOKEN && strcmp(tokens[k].content,".")==0)
        {

            vector<token> lhs = {tokens.begin()+begin,tokens.begin()+k};
            vector<token> rhs = {tokens.begin()+k+1,tokens.begin()+end+1};
            if(lhs.size()==0 || rhs.size()==0)
                parseError("SyntaxError","Invalid Syntax");
            Node* ast = NewNode(NodeType::memb);
            Node* line = NewNode(NodeType::line,to_string(tokens[0].ln));
            ast->childs.push_back(line);
            ast->childs.push_back(parseExpr(lhs));
            ast->childs.push_back(parseExpr(rhs));
            //rhs must be a function call as stated above

            if(ast->childs.back()->type!=NodeType::call)
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
        vector<token> dummyStmt;
        dummyStmt.push_back(make_token(ID_TOKEN,clone_str("println"),tokens[0].ln));
        dummyStmt.push_back(make_token(LParen_TOKEN,clone_str("("),0));
        dummyStmt.insert(dummyStmt.end(),tokens.begin(),tokens.end());
        dummyStmt.push_back(make_token(RParen_TOKEN,clone_str(")"),0));
        return parseExpr(dummyStmt);
    }
    parseError("SyntaxError","Unknown statement");
    return nullptr;//to avoid compiler warning otherwise the parseError function exits after printing error message
}

token else_token;
token elif_token;
token newline_token;
token bgscope_token;
token endscope_token;
token lp_token;

Node* Parser::parse(const vector<token>& tokens,int begin,int end)
{
    Node* ast = nullptr;
    else_token = make_token(KEYWORD_TOKEN, "else", 0);
    elif_token = make_token(KEYWORD_TOKEN, "else if", 0);
    newline_token = make_token(NEWLINE_TOKEN, "\n", 0);
    bgscope_token = make_token(L_CURLY_BRACKET_TOKEN, "{", 0);
    endscope_token = make_token(R_CURLY_BRACKET_TOKEN, "}", 0);
    lp_token = make_token(LParen_TOKEN, "(", 0);

    strip_newlines(tokens, begin, end);

    size_t tokens_size = end - begin + 1;
    if(tokens_size == 0 || begin > end)
        return NewNode(NodeType::EOP,"");

    Node* final = nullptr; // the final ast to return
    Node* e = nullptr;//e points to the lowest rightmost node of final
    line_num = 1;
    bool a,b,c;


    size_t k = begin;
    while(k<=end)
    {
        if(tokens[k].type== TokenType::NEWLINE_TOKEN || k == end)
        {
            if(k == end)
                k++;
            //vector<token> line = {tokens.begin()+start,tokens.begin()+k};
            size_t line_begin = begin;
            size_t line_end = k-1;
            size_t line_size = line_end - line_begin + 1;

            if(line_size == 0)
            {
                begin += 1;
                k+=1;
                continue;
            }
            line_num = tokens[line_begin].ln;
            //for ifelse and loop statements, we pop the curly bracket from line
            //for multiline expressions we don't
            //multiline expressions begin by adding '{' or '(' or '[' at the end
            //of a line that is not a conditional statement or loop stmt
            std::vector<token> multiline;
            bool use_multiline = false;
            if
            (
                ((a = tokens[line_end].type == TokenType::L_CURLY_BRACKET_TOKEN) ||
                (b = tokens[line_end].type == TokenType::LParen_TOKEN) ||
                (c = tokens[line_end].type == TokenType::BEGIN_LIST_TOKEN)) && line_size != 1
            )
            {
                if( tokens[line_begin].type != TokenType::KEYWORD_TOKEN ||
                    (tokens[line_begin].type == TokenType::KEYWORD_TOKEN &&
                    (strcmp(tokens[line_begin].content , "var") == 0 || strcmp(tokens[line_begin].content,"return")==0 || strcmp(tokens[line_begin].content,"yield") == 0)
                    )   
                )
                {
                    int idx = (int)k -1;
                    int rp = 0;
                    if(a)
                        rp = match_token(idx,TokenType::R_CURLY_BRACKET_TOKEN,tokens);
                    else if(b)
                        rp = match_token(idx,TokenType::RParen_TOKEN,tokens);
                    else if(c)
                        rp = match_token(idx,TokenType::END_LIST_TOKEN,tokens);
                    if(rp == -1)
                        parseError("SyntaxError","'"+(string)tokens[line_end].content+"' at the end of line is unmatched.");
                    

                    size_t i = 0;
                    line_end = rp;
                    for(i=begin;i<=(size_t)rp ;i++)
                    {
                        if(tokens[i].type != TokenType::NEWLINE_TOKEN)
                            multiline.push_back(tokens[i]);
                    }
                    //rp currently includes ending bracket on some line
                    //that whole line should be included
                    rp++;
                    while(rp <= end )
                    {
                        if(tokens[rp].type == NEWLINE_TOKEN)
                            break;
                        else
                            multiline.push_back(tokens[rp]);
                        rp+=1;
                        line_end++;
                    }
                    use_multiline = true;
                    line_begin = 0;
                    line_end = multiline.size()-1;
                    k = rp;//tokens before ith idx are handled
                }
                else if(a)//pop curly bracket only
                    line_end -=1 ;
                
            }

            line_size = line_end - line_begin +1;
            /*if(line_size==0)
            {
                begin+=1;
                k+=1;
                continue;
            }*/

            if(use_multiline)
                ast = parseStmt(multiline, line_begin,line_end);
            else
                ast = parseStmt(tokens,line_begin,line_end);

            if(ast->type==NodeType::IF)
            {
                token lptok;
                lptok.type = TokenType::LParen_TOKEN;
                lptok.content = "(";
                int if_begin = find_token_consecutive(bgscope_token,begin+line_size,end,tokens);
                int if_end;
                if(if_begin!=-1)
                {
                    if_end = match_token(if_begin,TokenType::R_CURLY_BRACKET_TOKEN,tokens);
                    if(if_end == -1)
                    {
                      line_num = tokens[if_begin].ln;
                      parseError("SyntaxError","Error expected '}' to match this.");
                    }
                }
                else
                {
                    if_begin = find_token_consecutive(newline_token,begin+line_size,end,tokens);
                    if_end = find_token(newline_token,if_begin+1,end,tokens);
                    line_num = tokens[line_begin].ln;
                    if(if_end==-1)
                        if_end = end+1;
                }

                if_begin += 1;
                if_end -= 1;

                vector<std::pair<int,int>> elifBlocks;
                vector<std::pair<int,int>> elifConditions;
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
                        line_num = tokens[elif_begin].ln;
                        parseError("SyntaxError","Error expected '(' after else if");
                    }
                    int l = match_token(p,TokenType::RParen_TOKEN,tokens);
                    elifConditions.push_back(std::pair<int,int>(p+1,l-1));
                    elif_begin = find_token_consecutive(bgscope_token,l+1,end,tokens);
                    if(elif_begin==-1)
                    {
                        elif_begin = find_token_consecutive(newline_token,l+1,end,tokens);
                        elif_end = find_token(newline_token,elif_begin+1,end,tokens);
                        line_num = tokens[p].ln;
                        elif_begin-=1;
                        if(elif_end == -1)
                            elif_end = end+1;

                    }
                    else
                        elif_end = match_token(elif_begin,TokenType::R_CURLY_BRACKET_TOKEN,tokens);//IMPORTANT
                    
                    elifBlocks.push_back(std::pair<int,int>(elif_begin+1,elif_end-1));
                    elif_begin = find_token_consecutive(elif_token,elif_end+1,end,tokens);
                    last_elif_end = elif_end-1;
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
                    line_num = tokens[else_begin].ln;
                    int e = else_begin;
                    else_begin = find_token_consecutive(bgscope_token,else_begin+1,end,tokens);
                    if(else_begin==-1)
                    {
                        else_begin = find_token_consecutive(newline_token,e+1,end,tokens);
                        if(else_begin == -1)
                            parseError("SyntaxError","Expected statement after else keyword");
                        else_end = find_token(newline_token,else_begin+1,end,tokens);
                        if(else_end == -1)
                            else_end = end+1;

                    }
                    else
                        else_end = match_token(else_begin,TokenType::R_CURLY_BRACKET_TOKEN,tokens);//IMPORTANT
                    else_begin += 1;
                    else_end -= 1;
                }
                if(foundelif && foundElse)
                {
                    ast->type = NodeType::IFELIFELSE;
                    for(int a=0;a<(int)elifConditions.size();a++)
                    {
                        vector<token> cond = {tokens.begin()+elifConditions[a].first,tokens.begin()+elifConditions[a].second+1};  
                        line_num = cond[0].ln;
                        ast->childs[1]->childs.push_back(parseExpr(cond));
                    }

                    bool ctxCopy = inif;
                    inif = true;
                    ast->childs.push_back(parse(tokens,if_begin,if_end));
                    inif = ctxCopy;

                    for(int a=0;a<(int)elifBlocks.size();a++)
                    {
                        int elif_begin = elifBlocks[a].first;
                        int elif_end = elifBlocks[a].second;
                        bool ctxCopy = inelif;
                        inelif = true;
                        Node* n = parse(tokens,elif_begin,elif_end);
                        inelif = ctxCopy;
                        ast->childs.push_back(n);
                    }
                    ctxCopy = inelse;
                    inelse = true;
                    ast->childs.push_back(parse(tokens,else_begin,else_end));
                    inelse = ctxCopy;
                    if(!final)
                    {
                        final = ast;
                        e = ast;
                    }
                    else
                    {
                        e->childs.push_back(ast);
                        e = e->childs.back();
                    }
                    begin = else_end+2;
                    k = else_end+1;
                }
                else if(foundelif && !foundElse)
                {
                    ast->type = NodeType::IFELIF;
                    for(int a=0;a<(int)elifConditions.size();a++)
                    {
                        vector<token> cond = {tokens.begin()+elifConditions[a].first,tokens.begin()+elifConditions[a].second+1};
                        line_num = cond[0].ln;
                        ast->childs[1]->childs.push_back(parseExpr(cond));
                    }
                    bool ctxCopy = inif;
                    inif = true;
                    ast->childs.push_back(parse(tokens,if_begin,if_end));
                    inif = ctxCopy;

                    for(int a=0;a<(int)elifBlocks.size();a++)
                    {
                        int elifblock_begin = elifBlocks[a].first;
                        int elifblock_end = elifBlocks[a].second;
                        bool ctxCopy = inelif;
                        inelif = true;
                        Node* n = parse(tokens,elifblock_begin,elifblock_end);
                        inelif = ctxCopy;
                        ast->childs.push_back(n);
                    }
                    if(!final)
                    {
                        final = ast;
                        e = ast;
                    }
                    else
                    {
                        e->childs.push_back(ast);
                        e = e->childs.back();
                    }
                    begin=elif_end+2;
                    k = elif_end+1;
                }
                else if(!foundelif && foundElse)
                {
                    ast->type = NodeType::IFELSE;
                    bool ctxCopy = inif;
                    inif = true;

                    ast->childs.push_back(parse(tokens,if_begin,if_end));
                    inif = ctxCopy;
                    ctxCopy = inelse;
                    inelse = true;

                    ast->childs.push_back(parse(tokens,else_begin,else_end));
                    inelse = ctxCopy;
                    if(!final)
                    {
                        final = ast;
                        e = ast;
                    }
                    else
                    {
                        e->childs.push_back(ast);
                        e = e->childs.back();
                    }
                    begin=else_end+2;
                    k = else_end+1;
                }
                else if(!foundelif && !foundElse)
                {
                    ast->type = NodeType::IF;
                    bool ctxCopy = inif;
                    inif = true;
                    ast->childs.push_back(parse(tokens,if_begin,if_end));
                    inif = ctxCopy;
                    if(!final)
                    {
                        final = ast;
                        e = ast;
                    }
                    else
                    {
                        e->childs.push_back(ast);
                        e = e->childs.back();
                    }
                    begin=if_end+2;
                    k = if_end+1;
                }
            }
            else if(ast->type==NodeType::FUNC)
            {
                int func_begin = find_token_consecutive(bgscope_token,begin+line_size,end,tokens);
                int func_end = -1;
                if(func_begin==-1)
                {
                    func_begin = find_token_consecutive(newline_token,begin+line_size,end,tokens);
                    if(func_begin==-1)
                        parseError("SyntaxError","Expected code block after function definition!");
                    func_end = find_token(newline_token,func_begin+1,end,tokens);
                    if(func_end==-1)
                        parseError("SyntaxError","Expected code block after function definition!");
                }
                else
                    func_end = match_token(func_begin,TokenType::R_CURLY_BRACKET_TOKEN,tokens);
                if(func_end==-1)
                    parseError("SyntaxError","Error missing '}' to end function definition.");
                if(func_begin==-1)
                    parseError("SyntaxError","Error expected code block or line after function definition!");
                func_begin += 1;
                func_end -= 1;

                if(infunc)
                    parseError("SyntaxError","Nested functions not allowed!");
                if(!isValidCtxForFunc())
                    parseError("SyntaxError","Local functions not allowed!");
                foundYield = false;
                string aux = currSym;
                if(!inclass)
                {
                    currSym = ast->childs[1]->val;
                    str_vector v;
                    str_vector_init(&v);
                    refgraph_emplace(refGraph,clone_str(ast->childs[1]->val.c_str()),v);
                }
                infunc = true;
                
                ast->childs.push_back(parse(tokens,func_begin,func_end));
                infunc = false;
                if(!inclass)
                    currSym = aux;

                if(foundYield)
                    ast->type = NodeType::CORO;

                if(!final)
                {
                    final = ast;
                    e = ast;
                }
                else
                {
                    e->childs.push_back(ast);
                    e = e->childs.back();
                }
                begin=func_end+2;
                k = func_end+1;
            }
            else if(ast->type==NodeType::CLASS || ast->type==NodeType::EXTCLASS)
            {
                int class_begin = find_token_consecutive(bgscope_token,begin+line_size,end,tokens);
                int class_end = -1;
                if(class_begin==-1)
                {
                    parseError("SyntaxError","Error expected brackets {} after class definition!");
                }
                else
                    class_end = match_token(class_begin,TokenType::R_CURLY_BRACKET_TOKEN,tokens);
                if(!atGlobalLevel())
                    parseError("SyntaxError","Class declartions must be at global scope or in a namespace!");
                class_begin += 1;
                class_end -= 1;
                inclass = true;
                string aux = currSym;
                currSym = ast->childs[1]->val;

                str_vector v;
                str_vector_init(&v);
                refgraph_emplace(refGraph,clone_str(ast->childs[1]->val.c_str()),v);
                if(ast->val=="class")
                    ast->childs.push_back(parse(tokens,class_begin,class_end));
                else
                    ast->childs.insert(ast->childs.begin()+2,parse(tokens,class_begin,class_end));
                //backtrack
                //important: change if planning to support nested classes
                inclass = false;
                currSym = aux;
                if(!final)
                {
                    final = ast;
                    e = ast;
                }
                else
                {
                    e->childs.push_back(ast);
                    e = e->childs.back();
                }
                begin=class_end+2;
                k = class_end+1;
            }
            else if(ast->type==NodeType::NAMESPACE)
            {
                int nm_begin = find_token_consecutive(bgscope_token,begin+line_size,end,tokens);
                int nm_end = -1;
                if(nm_begin==-1)
                {
                    line_num = tokens[begin].ln;
                    parseError("SyntaxError","Error expected {} after namespace keyword");
                }
                else
                    nm_end = match_token(nm_begin,TokenType::R_CURLY_BRACKET_TOKEN,tokens);
                if(!atGlobalLevel())
                    parseError("SyntaxError","Namespace declaration must be at global scope or within another namespace!");
                //vector<token> block = {tokens.begin()+j+1,tokens.begin()+i};
                nm_begin += 1;
                nm_end -= 1;

                const char* prefix = "";
                for(auto e: prefixes)
                    prefix = merge_str(prefix,e);
                prefix = merge_str(prefix,ast->childs[1]->val.c_str());
                prefix = merge_str(prefix,"::");
                prefixes.push_back(prefix);//prefix each identifier in namespace block
                //with "namespaceName::"
                ast->childs.push_back(parse(tokens,nm_begin,nm_end));
                prefixes.pop_back();
                if(!final)
                {
                    final = ast;
                    e = ast;
                }
                else
                {
                    e->childs.push_back(ast);
                    e = e->childs.back();
                }
                begin = nm_end+2;
                k = nm_end+1;
            }
            else if(ast->type==NodeType::WHILE || ast->type==NodeType::DOWHILE || ast->type==NodeType::FOR || ast->type == NodeType::DFOR || ast->type==NodeType::FOREACH)//loops
            {
                int loop_begin = find_token_consecutive(bgscope_token,begin+line_size,end,tokens);
                int loop_end;
                if(loop_begin==-1)
                {
                    loop_begin = find_token_consecutive(newline_token,begin+line_size,end,tokens);
                    loop_end = find_token(newline_token,loop_begin+1,end,tokens);
                    if(loop_end==-1 || (loop_begin<(int)tokens.size()-1 && tokens[loop_begin+1].type==END_TOKEN))
                        parseError("SyntaxError","Error expected a code block or line after "+ast->val);
                }
                else
                    loop_end = match_token(loop_begin,TokenType::R_CURLY_BRACKET_TOKEN,tokens);
                
                loop_begin += 1;
                loop_end -= 1;
                bool b = inloop;
                inloop = true;
                ast->childs.push_back(parse(tokens,loop_begin,loop_end));//inwhile = true
                inloop = b;
                if(!final)
                {
                    final = ast;
                    e = final;
                }
                else
                {
                    e->childs.push_back(ast);
                    e = e->childs.back();
                }
                begin=loop_end+2;
                k = loop_end+1;
            }
            else if(ast->type==NodeType::TRYCATCH)//try catch
            {
                int try_begin = find_token_consecutive(bgscope_token,begin+line_size,end,tokens);
                int try_end = -1;
                if(try_begin==-1)
                {
                    try_begin = find_token_consecutive(newline_token,begin+line_size,end,tokens);
                    try_end = find_token(newline_token,try_begin+1,end,tokens);
                    if(try_end==-1)
                        parseError("SyntaxError","Error expected a code block or line after try!");
                }
                else
                    try_end = match_token(try_begin,TokenType::R_CURLY_BRACKET_TOKEN,tokens);

                try_begin += 1;
                try_end -= 1;
                /*if(block.size()==1 && block[0].type==KEYWORD_TOKEN && (strcmp(block[0].content,"endfor")==0 || strcmp(block[0].content,"endwhile")==0 || strcmp(block[0].content,"endnm")==0 || strcmp(block[0].content,"endclass")==0 || strcmp(block[0].content,"endfunc")==0 || strcmp(tokens[0].content,"endtry")==0 || strcmp(tokens[0].content,"endcatch")==0 || strcmp(block[0].content,"endelse")==0 || strcmp(block[0].content,"endelif")==0 ||  strcmp(block[0].content,"endif") == 0))
                {
                    parseError("SyntaxError","Error use brackets {} after "+ast->val);
                }
                stripNewlines(block);*/


                token CATCH_TOK;
                CATCH_TOK.type = TokenType::KEYWORD_TOKEN;
                CATCH_TOK.content = "catch";
                const char* catchId ;
                int catch_begin = find_token_consecutive(CATCH_TOK,try_end+2,end,tokens);
                int catch_end = -1;
                if(catch_begin!=-1)
                {
                    line_num = tokens[catch_begin].ln;
                    if(catch_begin+4 > end)
                        parseError("SyntaxError","Invalid Syntax");
                    catch_begin+=1;
                    if(tokens[catch_begin].type!=TokenType::LParen_TOKEN)
                        parseError("SyntaxError","Invalid Syntax");
                    catch_begin+=1;
                    if(tokens[catch_begin].type!=TokenType::ID_TOKEN)
                        parseError("SyntaxError","Invalid Syntax");
                    catchId = tokens[catch_begin].content;
                    catch_begin+=1;
                    if(tokens[catch_begin].type!=TokenType::RParen_TOKEN)
                        parseError("SyntaxError","Invalid Syntax");
                    catch_begin+=1;
                    catch_begin = find_token_consecutive(bgscope_token,catch_begin,end,tokens);
                    int e = catch_begin;
                    if(catch_begin==-1)
                    {
                        catch_begin = find_token_consecutive(newline_token,e+1,end,tokens);
                        catch_end = find_token(newline_token,catch_begin+1,end,tokens);
                        if(catch_end==-1 || (catch_end<(int)tokens.size()-1 && tokens[catch_end+1].type==END_TOKEN))
                            parseError("SyntaxError","Error expected code block or line after catch");
                    }
                    else
                        catch_end = match_token(catch_begin,TokenType::R_CURLY_BRACKET_TOKEN,tokens);
                }
                else
                {
                    parseError("SyntaxError","Error expected catch block after try{}!");
                }
                catch_begin += 1;
                catch_end -= 1;
                ast->childs.push_back(NewNode(NodeType::ID,catchId));
                bool ctxCopy = intry;
                intry = true;
                ast->childs.push_back(parse(tokens,try_begin,try_end));
                intry = ctxCopy;
                ctxCopy = incatch;
                incatch = true;
                ast->childs.push_back(parse(tokens,catch_begin,catch_end));
                incatch = ctxCopy;
                if(!final)
                {
                    final = ast;
                    e = final;
                }
                else
                {
                    e->childs.push_back(ast);
                    e = e->childs.back();
                }
                begin=catch_end+2;
                k = catch_end+1;
            }
            else if(ast->type==NodeType::BREAK || ast->type==NodeType::CONTINUE)
            {
                if(!inloop)
                    parseError("SyntaxError","Error use of break or continue not allowed outside loop!");
                if(!final)
                {
                    final = ast;
                    e = final;
                }
                else
                {
                    e->childs.push_back(ast);
                    e = e->childs.back();
                }
                begin = k+1;
            }
            else if(ast->type==NodeType::RETURN_NODE)
            {
                if(!infunc)
                {
                    parseError("SyntaxError","Error use of return statement outside functon!");
                }
                if(!final)
                {
                    final = ast;
                    e = final;
                }
                else
                {
                    e->childs.push_back(ast);
                    e = e->childs.back();
                }
                begin = k+1;
            }
            else if(ast->type==NodeType::import)
            {
            string str = ast->childs[1]->val;
            if(ast->childs[1]->type!=NodeType::ID)
            {
                Node* A;
                if( str_vector_search(files,str.c_str()) != -1)
                {
                    A = NewNode(NodeType::file);
                    A->childs.push_back(NewNode(NodeType::line,to_string(line_num)));
                    A->childs.push_back(NewNode(NodeType::STR,str));
                    A->childs.push_back(NewNode(NodeType::EOP));
                }
                else
                {
                    FILE* file = fopen(str.c_str(),"r");
                    if(!file)
                        parseError("ImportError",strerror(errno));
                    str_vector_push(files,clone_str(str.c_str()));
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
                    fclose(file);
                    str_vector_push(sources,clone_str(src.c_str()));
                    lexer lex;
                    zuko_src tmp;
                    zuko_src_init(&tmp);
                    zuko_src_add_file(&tmp, clone_str(filename.c_str()),clone_str(src.c_str()));
                    //tmp.addFile(filename,src);
                    vector<token> tokens = lexer_generateTokens(&lex,&tmp);
                    bool empty_file = false;
                    if(tokens.size() == 1 && tokens[0].type==END_TOKEN)
                      empty_file = true;
                    if(lex.hadErr)
                    {
                        filename = F;
                        line_num = K;
                        //error was already printed
                        exit(1);
                    }
                    A = NewNode(NodeType::file);
                    A->childs.push_back(NewNode(NodeType::line,to_string(K)));
                    A->childs.push_back(NewNode(NodeType::STR,str));
                    Node* subast;
                    if(empty_file)
                        subast = NewNode(NodeType::EOP,"EOP");
                    else
                        subast = parse(tokens,0,tokens.size()-1);
                    A->childs.push_back(subast);
                    filename = F;
                    line_num = K;

                }
                if(!final)
                {
                    final = A;
                    e = A;
                }
                else
                {
                    e->childs.push_back(A);
                    e = A;
                }
                begin = k+1;
                delete_ast(ast);
            }
            else
            {
                if(!final)
                {
                    final = ast;
                    e = final;
                }
                else
                {
                    e->childs.push_back(ast);
                    e = e->childs.back();
                }
                begin = k+1;
            }
            }
            else
            {
                if(!final)
                {
                    final = ast;
                    e = final;
                }
                else
                {
                    e->childs.push_back(ast);
                    e = e->childs.back();
                }
                begin = k+1;
            }
        }
        k+=1;
    }
    e->childs.push_back(NewNode(NodeType::EOP));
    return final;
}

