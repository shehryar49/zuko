#include "parser.h"
#include "ast.h"
#include "lexer.h"
#include <string.h>
#include "convo.h"
#include "token.h"
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
void stripNewlines(vector<Token>& tokens)
{
    while(tokens.size()>0 && tokens[0].type==NEWLINE_TOKEN )
        tokens.erase(tokens.begin());
    while(tokens.size()>0 && tokens[tokens.size()-1].type==NEWLINE_TOKEN )
        tokens.pop_back();
}
//Function to find match for tokens like '(' '[' '{'
int matchToken(int k,TokenType end,const vector<Token>& tokens)
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
int matchTokenRight(int k,TokenType end,const vector<Token>& tokens)
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

int findToken(Token t,int start,const vector<Token>& tokens)
{
   int l = tokens.size();
   while(start<l)
   {
     if(tokens[start].type==t.type && tokens[start].content==t.content)
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
    //Copies one tree structure value by value( deep copy )
    if(!src)
        return;
    dest->val = src->val;
    int k = 0;
    int l = src->childs.size();
    for(k=0;k<l;k+=1)
    {
        Node* n = new Node;
        CopyAst(n,src->childs[k]);
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
void printAST(Node* n,int spaces)
{
   if(!n)
   {

     return;
   }
   printf("|%s %s\n",NodeTypeNames[(int)n->type],n->val.c_str());
   spaces+=2;
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
    int l = tokens.size();
    for(int k=start;k<l;k+=1)
    {
        if(tokens[k].type==t.type && tokens[k].content==t.content)
          return k;
        else if(tokens[k].type== TokenType::NEWLINE_TOKEN);
        else
          return -1;
    }
    return -1;
}


void Parser::init(string fname,ProgramInfo& p)
{
    num_of_constants = &p.num_of_constants;
    refGraph = &p.refGraph;
    files = &p.files;
    sources = &p.sources;
    filename = fname;
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
}
void Parser::parseError(string type,string msg)
{
    printf("\nFile %s\n",filename.c_str());
    printf("%s at line %zu\n",type.c_str(),line_num);
    auto it = std::find(files->begin(),files->end(),filename);
    size_t i = it-files->begin();
    string source_code = (*sources)[i];
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
    printf("%s\n",lstrip(line).c_str());
    printf("%s\n",msg.c_str());
    if(REPL_MODE)
        REPL();
    exit(1);
}
bool Parser::addSymRef(string name)
{
    if(name == currSym) // no self loops
        return false;
    if(refGraph->find(name)==refGraph->end())
        return false;

    // printf("adding edge %s to %s\n",currSym.c_str(),name.c_str());
    vector<string>& neighbours = (*refGraph)[currSym];
    if(std::find(neighbours.begin(),neighbours.end(),name) == neighbours.end())
        neighbours.push_back(name);
    return true;
}
Node* Parser::parseExpr(const vector<Token>& tokens)
{
    if(tokens.size()==0)
        parseError("SyntaxError","Invalid Syntax");

    //Tokens.size is never zero
    Node* ast = nullptr;

    if(tokens.size()==1)
    {
        if(tokens[0].type== TokenType::KEYWORD_TOKEN && tokens[0].content=="nil" )
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
                aux = (*it)+tokens[0].content;
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
            if(findToken(tokens[0],0,known_constants)==-1)
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
            int i = matchTokenRight(k,TokenType::LParen_TOKEN,tokens);
            if(i==-1)
                parseError("SyntaxError","Invalid Syntax");
            k = i;
        }
        else if(tokens[k].type== TokenType::END_LIST_TOKEN)
        {
            int i = matchTokenRight(k,TokenType::BEGIN_LIST_TOKEN,tokens);
            if(i==-1)
                parseError("SyntaxError","Invalid Syntax");
            k=i;
        }
        else if(tokens[k].type== TokenType::R_CURLY_BRACKET_TOKEN)
        {
            int i = matchTokenRight(k,TokenType::L_CURLY_BRACKET_TOKEN,tokens);
            if(i==-1)
                parseError("SyntaxError","Invalid Syntax");
            k=i;
        }
        else if(tokens[k].type== TokenType::OP_TOKEN && ((m = std::find(op_set.begin(),op_set.end(),tokens[k].content))!=op_set.end()) && k!=0)
        {
            vector<Token> lhs = {tokens.begin(),tokens.begin()+k};
            vector<Token> rhs = {tokens.begin()+k+1,tokens.end()};
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

        if(tokens[0].content=="-")
        {
        Node* ast = NewNode(NodeType::neg);
        vector<Token> e = {tokens.begin()+1,tokens.end()};
        if(e.size()==0)
            parseError("SyntaxError","Invalid Syntax");
        size_t sz = known_constants.size();
        Node* E = parseExpr(e);
        if(known_constants.size()==sz+1 && e.size()==1 && (known_constants[sz].type==FLOAT_TOKEN || known_constants[sz].type==NUM_TOKEN))
        {
            known_constants[sz].content = "-"+known_constants[sz].content;
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
        else if(tokens[0].content=="+")
        {
        vector<Token> e = {tokens.begin()+1,tokens.end()};
        if(e.size()==0)
            parseError("SyntaxError","Invalid Syntax");
        return (parseExpr(e));
        }
        else if(tokens[0].content=="~")
        {
        Node* ast = NewNode(NodeType::complement);
        vector<Token> e = {tokens.begin()+1,tokens.end()};
        if(e.size()==0)
            parseError("SyntaxError","Invalid Syntax");
        ast->childs.push_back(parseExpr(e));
        return ast;
        }
        else if(tokens[0].content=="!")
        {
        Node* ast = NewNode(NodeType::NOT);
        vector<Token> e = {tokens.begin()+1,tokens.end()};
        ast->childs.push_back(parseExpr(e));
        return ast;
        }
    }
    if(tokens[tokens.size()-1].type==END_LIST_TOKEN)// evaluate indexes or lists
    {
        int i = matchTokenRight((int)tokens.size()-1,TokenType::BEGIN_LIST_TOKEN,tokens);
        if(i==-1)
            parseError("SyntaxError","Invalid Syntax");
        if(i!=0)
        {
            vector<Token> toindex = {tokens.begin(),tokens.begin()+i};
            vector<Token> index = {tokens.begin()+i+1,tokens.end()-1};

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
            vector<Token> t;
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
                int R = matchToken(k,TokenType::END_LIST_TOKEN,tokens);
                if(R>(L) || R==-1)
                    parseError("SyntaxError","Invalid Syntax");
                vector<Token> sublist = {tokens.begin()+k,tokens.begin()+R+1};
                t.insert(t.end(),sublist.begin(),sublist.end());
                k = R;
            }
            else if(tokens[k].type== TokenType::L_CURLY_BRACKET_TOKEN)
            {
                int R = matchToken(k,TokenType::R_CURLY_BRACKET_TOKEN,tokens);
                if(R>L || R==-1)
                    parseError("SyntaxError","Invalid Syntax");
                vector<Token> subdict = {tokens.begin()+k,tokens.begin()+R+1};
                t.insert(t.end(),subdict.begin(),subdict.end());
                k = R;
            }
            else if(tokens[k].type==TokenType::ID_TOKEN && tokens[k+1].type==TokenType::LParen_TOKEN)
            {
                int R = matchToken(k,TokenType::RParen_TOKEN,tokens);
                if(R>L || R==-1)
                    parseError("SyntaxError","Invalid Syntax");
                vector<Token> subfn = {tokens.begin()+k,tokens.begin()+R+1};
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
        int i = matchTokenRight(k,TokenType::LParen_TOKEN,tokens);
        if(i==-1)
            parseError("SyntaxError","Invalid Syntax");
        k = i;
        }
        else if(tokens[k].type== TokenType::END_LIST_TOKEN)
        {
            int i = matchToken(k,TokenType::BEGIN_LIST_TOKEN,tokens);
            if(i==-1)
                parseError("SyntaxError","Invalid Syntax");
            k=i;
        }
        else if(tokens[k].type== TokenType::R_CURLY_BRACKET_TOKEN)
        {
            int i = matchTokenRight(k,TokenType::L_CURLY_BRACKET_TOKEN,tokens);
            if(i==-1)
                parseError("SyntaxError","Invalid Syntax");
            k = i;
        }
        else if(tokens[k].type== TokenType::OP_TOKEN && (tokens[k].content=="."))
        {
        vector<Token> lhs = {tokens.begin(),tokens.begin()+k};
        vector<Token> rhs = {tokens.begin()+k+1,tokens.end()};
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
            vector<Token> Key;
            vector<Token> Value;
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
                int R = matchToken(k,TokenType::END_LIST_TOKEN,tokens);
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
                int R = matchToken(k,TokenType::R_CURLY_BRACKET_TOKEN,tokens);
                if(R==(int)tokens.size()-1)
                    parseError("SyntaxError","Invalid Syntax");

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
            }
            else if(tokens[k].type==TokenType::ID_TOKEN && tokens[k+1].type==TokenType::LParen_TOKEN)
            {
                int R = matchToken(k,TokenType::RParen_TOKEN,tokens);
                if(R>((int)tokens.size()-2) || R==-1)
                    parseError("SyntaxError","Invalid Syntax");
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
        int i = matchToken(1,TokenType::RParen_TOKEN,tokens);
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
                aux = (*it)+tokens[0].content;
                if((done = addSymRef(aux)))
                    break;
                it++;
            }
            vector<Token> T;
            vector<Token> Args = {tokens.begin()+2,tokens.end()-1};
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
                    int i = matchToken(k,TokenType::END_LIST_TOKEN,Args);
                    if(i==-1)
                        parseError("SyntaxError","Invalid Syntax");
                    vector<Token> LIST =  {Args.begin()+k,Args.begin()+i+1};
                    T.insert(T.end(),LIST.begin(),LIST.end());
                    k = i;
                }
                else if(Args[k].type== TokenType::LParen_TOKEN)
                {
                    int i = matchToken(k,TokenType::RParen_TOKEN,Args);
                    if(i==-1)
                        parseError("SyntaxError","Invalid Syntax");
                    vector<Token> P =  {Args.begin()+k,Args.begin()+i+1};
                    T.insert(T.end(),P.begin(),P.end());
                    k = i;
                }
                else if(Args[k].type== TokenType::L_CURLY_BRACKET_TOKEN)
                {
                    int i = matchToken(k,TokenType::R_CURLY_BRACKET_TOKEN,Args);
                    if(i==-1)
                        parseError("SyntaxError","Invalid Syntax");
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
        }//
    }
    ///////////////////
    //////////////
    if(tokens[0].type== TokenType::LParen_TOKEN)
    {
        int i = matchToken(0,TokenType::RParen_TOKEN,tokens);
        if(i==-1)
            parseError("SyntaxError","Invalid Syntax");
        if(i==(int)tokens.size()-1)
        {
            vector<Token> e = {tokens.begin()+1,tokens.end()-1};
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
            expr+= "\"" + e.content + "\"";
        else
            expr+=e.content;
    }
    if(!multiline)
        parseError("SyntaxError","Invalid syntax: "+expr);
    else
        parseError("SyntaxError","Expression spans over multiple lines");  
    return ast;
}

Node* Parser::parseStmt(vector<Token> tokens)
{
if(tokens.size()==0)
    parseError("SyntaxError","Invalid Syntax");

if(tokens.size()==1 && (tokens[0].content=="endfunc" || tokens[0].content=="endnm" || tokens[0].content=="endtry" || tokens[0].content=="endcatch" || tokens[0].content=="endfor" || tokens[0].content=="endelse" || tokens[0].content=="endwhile" || tokens[0].content =="endclass" || tokens[0].content=="endelif"  || tokens[0].content=="endif") && tokens[0].type==KEYWORD_TOKEN)
{
    return NewNode(NodeType::end,tokens[0].content);
}
if(tokens.size()==1 && tokens[0].type==KEYWORD_TOKEN && tokens[0].content=="gc")
    return NewNode(NodeType::gc);
bool isPrivate = false;
if(tokens[0].type==TokenType::KEYWORD_TOKEN && tokens[0].content=="private")
{
    isPrivate = true;
    tokens.erase(tokens.begin());
    if(!inclass)
        parseError("SyntaxError","Error use of keyword private outside class!");
    if(tokens[0].type==TokenType::KEYWORD_TOKEN && tokens[0].content=="var")
    {

    }
    else if(tokens[0].type==TokenType::KEYWORD_TOKEN && tokens[0].content=="function")
    {

    }
    else
    parseError("SyntaxError","Invalid use of keyword private\n");
}
else if(tokens[0].type==TokenType::KEYWORD_TOKEN && tokens[0].content=="public")
{
    isPrivate = false;
    tokens.erase(tokens.begin());
    if(!inclass)
        parseError("SyntaxError","Error use of keyword public outside class!");
    if(tokens[0].type==TokenType::KEYWORD_TOKEN && tokens[0].content=="var")
    {

    }
    else if(tokens[0].type==TokenType::KEYWORD_TOKEN && tokens[0].content=="function")
    {

    }
    else
    parseError("SyntaxError","Invalid use of keyword public");
}

if(tokens[0].type== TokenType::KEYWORD_TOKEN  && tokens[0].content=="var")
{
    //declare statement
    if(tokens.size()<4) // there must be minimum four tokens var x  (simplest case)
    {
        parseError("SyntaxError","Invalid declare statement!");
    }
    if(tokens[1].type!= TokenType::ID_TOKEN || (tokens[2].type!= TokenType::OP_TOKEN &&  tokens[2].content!="="))
    {
    parseError("SyntaxError","invalid declare statement!");
    }
    Node* ast = NewNode(NodeType::declare);
    Node* n = NewNode(NodeType::line,to_string(tokens[0].ln));
    ast->childs.push_back(n);
    if(tokens[1].content.find("::")!=string::npos)
        parseError("SyntaxError","Invalid Syntax");
    string fnprefix = prefixes.back();
    if(atGlobalLevel())
        tokens[1].content = fnprefix+tokens[1].content;
    if(isPrivate)
        tokens[1].content = "@"+tokens[1].content;
    //ast->childs.push_back(NewNode(NodeType::ID,tokens[1].content));
    ast->val = tokens[1].content;
    vector<Token> expr = {tokens.begin()+3,tokens.end()};
    if(expr[0].type==KEYWORD_TOKEN && expr[0].content == "yield")
    {
        foundYield = true;
        expr = {expr.begin()+1,expr.end()};
        ast->childs.push_back(NewNode(NodeType::YIELD));
        if(expr.size()==0)
          parseError("SyntaxError","Error expected expression after return keyword");
        Node* n = NewNode(NodeType::line ,to_string(tokens[0].ln));
        ast->childs.back()->childs.push_back(n);
        ast->childs.back()->childs.push_back(parseExpr(expr));
        return ast;
    }
    ast->childs.push_back(parseExpr(expr));
    return ast;
}
if(tokens[0].type== TokenType::EOP_TOKEN && tokens.size()==1)
{
    Node* ast = NewNode(NodeType::EOP);
    return ast;
}
if(tokens[0].type== TokenType::KEYWORD_TOKEN && (tokens[0].content=="break" || tokens[0].content=="continue") && tokens.size()==1)
{
    Node* ast = NewNode(tokens[0].content == "break" ? NodeType::BREAK : NodeType::CONTINUE);
    ast->childs.push_back(NewNode(NodeType::line,to_string(tokens[0].ln)));
    return ast;
}
if(tokens[0].type== TokenType::ID_TOKEN)
{
    if(tokens.size()>=3)
    {
    if(tokens[1].type== TokenType::LParen_TOKEN && tokens[tokens.size()-1].type== TokenType::RParen_TOKEN && matchToken(1,TokenType::RParen_TOKEN,tokens)==((int)tokens.size()-1))
    {
        bool wrapInPrint = false;
        if(REPL_MODE && atGlobalLevel() && tokens[0].content!="print" && tokens[0].content!="println" && tokens[0].content!="printf")
            wrapInPrint = true;
        
        Node* ast = NewNode(NodeType::call);
        Node* n = NewNode(NodeType::line,to_string(tokens[0].ln));
        ast->childs.push_back(n);
        ast->childs.push_back(NewNode(NodeType::ID,tokens[0].content));
        //if()
        auto it = prefixes.rbegin();
        bool done = false;
        while(it != prefixes.rend())
        {
            aux = (*it)+tokens[0].content;
            if((done = addSymRef(aux)))
                break;
            it++;
        }
        Node* args = NewNode(NodeType::args);
        if(tokens.size()==3)
        {
            ast->childs.push_back(args);    
        }
        else
        {
            vector<Token> T;
            vector<Token> Args = {tokens.begin()+2,tokens.end()-1};
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
                    int i = matchToken(k,TokenType::END_LIST_TOKEN,Args);
                    if(i==-1)
                        parseError("SyntaxError","Invalid Syntax");
                    vector<Token> P = {Args.begin()+k,Args.begin()+i+1};
                    T.insert(T.end(),P.begin(),P.end());
                    k = i;
                }
                else if(Args[k].type== TokenType::L_CURLY_BRACKET_TOKEN)
                {
                    int i = matchToken(k,TokenType::R_CURLY_BRACKET_TOKEN,Args);
                    if(i==-1)
                        parseError("SyntaxError","Invalid Syntax");
                    vector<Token> P = {Args.begin()+k,Args.begin()+i+1};
                    T.insert(T.end(),P.begin(),P.end());
                    k = i;
                }
                else if(Args[k].type== TokenType::LParen_TOKEN)
                {
                    int i = matchToken(k,TokenType::RParen_TOKEN,Args);
                    if(i==-1)
                        parseError("SyntaxError","Invalid Syntax");
                    vector<Token> P = {Args.begin()+k,Args.begin()+i+1};
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

if(tokens[0].type==TokenType::KEYWORD_TOKEN && tokens[0].content=="class")
{
    if(tokens.size()<2)
        parseError("SyntaxError","Invalid Syntax");
    if(tokens[1].type!=ID_TOKEN)
        parseError("SyntaxError","Invalid Syntax");
    bool extendedClass=false;
    if(tokens.size()>=4)
    {
    if(tokens[2].type!=TokenType::KEYWORD_TOKEN || tokens[3].type!=TokenType::ID_TOKEN)
        parseError("SyntaxError","Invalid Syntax");
    if(tokens[2].content!="extends")
        parseError("SyntaxError","Invalid Syntax");
    extendedClass = true;
    }
    else if(tokens.size()==2)
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
    if(tokens[1].content.find("::")!=string::npos)
        parseError("SyntaxError","Invalid Syntax");
    string fnprefix = prefixes.back();

    if(atGlobalLevel())
        tokens[1].content = fnprefix+tokens[1].content;
    ast->childs.push_back(NewNode(NodeType::ID,tokens[1].content));
    if(extendedClass)
    {
        tokens = {tokens.begin()+3,tokens.end()};
        ast->childs.push_back(parseExpr(tokens));
    }
    return ast;
}
if(tokens[0].type== TokenType::KEYWORD_TOKEN && (tokens[0].content=="while" || tokens[0].content=="dowhile"))
{
    if(tokens.size()>=4)
    {
        if(tokens[1].type== TokenType::LParen_TOKEN  && tokens.back().type== TokenType::RParen_TOKEN)
        {
            vector<Token> expr = {tokens.begin()+2,tokens.end()-1};
            Node* ast = NewNode(tokens[0].content == "while" ? NodeType::WHILE : NodeType::DOWHILE);
            Node* n = NewNode(NodeType::line ,to_string(tokens[0].ln));
            ast->childs.push_back(n);
            ast->childs.push_back(parseExpr(expr));
            return ast;
        }
    }
}
if(tokens[0].type==TokenType::KEYWORD_TOKEN && tokens[0].content=="for")
{
    if(tokens.size()<3)
        parseError("SyntaxError","Invalid Syntax");
    if(tokens[1].type!=TokenType::LParen_TOKEN || tokens.back().type!=TokenType::RParen_TOKEN)
    {
        parseError("SyntaxError","Invalid Syntax");
    }
    tokens = {tokens.begin()+2,tokens.end()-1};

    Token t;
    t.type = TokenType::KEYWORD_TOKEN;
    t.content = "to";
    int i = findToken(t,0,tokens);
    bool dtoLoop = false;
    if(i==-1)
    {
        t.content = "dto";
        i = findToken(t,0,tokens);
        if(i == -1)
            parseError("SyntaxError","Invalid Syntax");
        dtoLoop = true;
    }
    vector<Token> init = {tokens.begin(),tokens.begin()+i};

    int p = i;
    t.content = "step";
    t.type = TokenType::KEYWORD_TOKEN;
    i = findToken(t,i+1,tokens);
    vector<Token> endpoint;
    vector<Token> inc;
    if(i==-1)//step is optional
    {
        endpoint = {tokens.begin()+p+1,tokens.end()};
        Token one;
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
        inc = {tokens.begin()+i+1,tokens.end()};
    }
    bool decl = false;
    if(init[0].type==TokenType::KEYWORD_TOKEN && init[0].content=="var" )
    {
        init.erase(init.begin());
        decl = true;
    }
    if(init[0].type==TokenType::ID_TOKEN && init[1].type==TokenType::OP_TOKEN && init[1].content=="=")
    {
        vector<Token> initExpr = {init.begin()+2,init.end()};
        Node* ast;
        if(dtoLoop)
            ast = NewNode(NodeType::DFOR);
        else
            ast = NewNode(NodeType::FOR);
        Node* n = NewNode(NodeType::line,to_string(tokens[0].ln));
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
if(tokens[0].type==TokenType::KEYWORD_TOKEN && tokens[0].content=="foreach")
{
    if(tokens.size()<3)
        parseError("SyntaxError","Invalid Syntax");

    if(tokens[1].type!=TokenType::LParen_TOKEN || tokens.back().type!=TokenType::RParen_TOKEN)
        parseError("SyntaxError","Invalid Syntax");
    tokens = {tokens.begin()+2,tokens.end()-1};
    if(tokens.size()<4)
        parseError("SyntaxError","Invalid Syntax");
    if(tokens[0].type!=TokenType::KEYWORD_TOKEN || tokens[0].content!="var" || tokens[1].type!=TokenType::ID_TOKEN || tokens[2].type!=TokenType::COLON_TOKEN)
        parseError("SyntaxError","Invalid Syntax");
    vector<Token> expr = {tokens.begin()+3,tokens.end()};

    Node* exprAST = parseExpr(expr);
    Node* ast = NewNode(NodeType::FOREACH);
    Node* n = NewNode(NodeType::line,to_string(tokens[0].ln));
    ast->childs = {n,NewNode(NodeType::ID,tokens[1].content),exprAST};
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
        parseError("SyntaxError","Invalid Syntax");
    if(tokens[1].type!=ID_TOKEN)
        parseError("SyntaxError","Invalid Syntax");
    Node* ast = NewNode(NodeType::NAMESPACE);
    Node* n = NewNode(NodeType::line,to_string(tokens[0].ln));
    ast->childs.push_back(n);
    string fnprefix;
    if(tokens[1].content.find("::")!=string::npos)
        parseError("SyntaxError","Invalid Syntax");

    ast->childs.push_back(NewNode(NodeType::ID,tokens[1].content));
    return ast;
}
if(tokens[0].type== TokenType::KEYWORD_TOKEN && tokens[0].content=="if")
{
    if(tokens.size()>=4)
    {
    if(tokens[1].type== TokenType::LParen_TOKEN  && tokens.back().type== TokenType::RParen_TOKEN)
    {
        vector<Token> expr = {tokens.begin()+2,tokens.end()-1};
        Node* ast = NewNode(NodeType::IF);
        Node* n = NewNode(NodeType::line,to_string(tokens[0].ln));
        ast->childs.push_back(n);
        Node* conditions = NewNode(NodeType::conditions);
        conditions->childs.push_back(parseExpr(expr));
        ast->childs.push_back(conditions);
        return ast;
    }
    }
}
if(tokens[0].type== TokenType::KEYWORD_TOKEN && tokens[0].content=="import")
    {
        Node* ast = NewNode(NodeType::import);
        vector<Token> t;
        if(tokens.size()==2)
        {
        if(tokens[1].type==ID_TOKEN)
        {
            t.push_back(tokens[1]);
            Node* n = NewNode(NodeType::line,to_string(tokens[0].ln));
            ast->childs.push_back(n);
            ast->childs.push_back(parseExpr(t));
            return ast;
        }
        if(tokens[1].type== TokenType::STRING_TOKEN)
        {
            t.push_back(tokens[1]);
            Node* n = NewNode(NodeType::line,to_string(tokens[0].ln));
            ast->childs.push_back(n);
            ast->childs.push_back(NewNode(NodeType::STR,tokens[1].content));
            return ast;
        }
        else
        {
                //line_num = tokens[1].ln;
                parseError("SyntaxError","Invalid Syntax");
        }
        }
        else if(tokens.size()==4 && tokens[2].content == "as")
        {
        if(tokens[1].type!=ID_TOKEN || tokens[2].type!=KEYWORD_TOKEN || tokens[2].content!="as" || tokens[3].type!=ID_TOKEN)
        {
            parseError("SyntaxError","Invalid Syntax");
        }
        ast->type = NodeType::importas;
        t.push_back(tokens[1]);
        Node* n = NewNode(NodeType::line,to_string(tokens[0].ln));
        ast->childs.push_back(n);
        ast->childs.push_back(parseExpr(t));
        ast->childs.push_back(NewNode(NodeType::ID,tokens[3].content));
        return ast;
        }
        else if(tokens.size()==4)
        {
            if(tokens[1].type!=ID_TOKEN || tokens[1].content!="std" || tokens[2].type!=OP_TOKEN || tokens[2].content!="/" || tokens[3].type!=ID_TOKEN )
            parseError("SyntaxError","Invalid Syntax");
        #ifdef _WIN32
        tokens[3].content="C:\\zuko\\std\\"+tokens[3].content+".zk";
        #else
        tokens[3].content="/opt/zuko/std/"+tokens[3].content+".zk";
        #endif
        Node* n = NewNode(NodeType::line,to_string(tokens[0].ln));
        ast->childs.push_back(n);
        ast->childs.push_back(NewNode(NodeType::STR,tokens[3].content));
        return ast;
        }
        else
        {
            parseError("SyntaxError","Invalid Syntax");
        }
    }
if(tokens[0].type== TokenType::KEYWORD_TOKEN && tokens[0].content=="return")
    {
        Node* ast = NewNode(NodeType::RETURN_NODE);
        vector<Token> t = {tokens.begin()+1,tokens.end()};
        if(t.size()==0)
            parseError("SyntaxError","Error expected expression after return keyword");
        Node* n = NewNode(NodeType::line,to_string(tokens[0].ln));
        ast->childs.push_back(n);
        ast->childs.push_back(parseExpr(t));
        return ast;
    }
if(tokens[0].type== TokenType::KEYWORD_TOKEN && tokens[0].content=="yield")
    {
    foundYield = true;
    Node* ast = NewNode(NodeType::YIELD);
    vector<Token> t = {tokens.begin()+1,tokens.end()};
    if(t.size()==0)
        parseError("SyntaxError","Error expected expression after yield keyword");
    Node* n = NewNode(NodeType::line,to_string(tokens[0].ln));
    ast->childs.push_back(n);
    ast->childs.push_back(parseExpr(t));
    return ast;
    }
if(tokens[0].type== TokenType::KEYWORD_TOKEN && tokens[0].content=="function")
{
    if(tokens.size()<4)
        parseError("SyntaxError","Invalid Syntax");
    if(tokens.back().type==L_CURLY_BRACKET_TOKEN)
        tokens.pop_back();
    if(tokens[1].type!=ID_TOKEN || tokens.back().type!=TokenType::RParen_TOKEN)
        parseError("SyntaxError","Invalid Syntax");
    if(tokens[1].content.find("::")!=string::npos)
        parseError("SyntaxError","Invalid Syntax");
    string fnprefix = prefixes.back();
    if(atGlobalLevel())
        tokens[1].content = fnprefix+tokens[1].content;
    if(isPrivate)
        tokens[1].content = "@" + tokens[1].content;
    Node* ast = NewNode(NodeType::FUNC);
    Node* n = NewNode(NodeType::line,to_string(tokens[0].ln));
    ast->childs.push_back(n);
    ast->childs.push_back(NewNode(NodeType::ID,tokens[1].content));
    ast->childs.push_back(NewNode(NodeType::args));
    if(tokens.size()==4)
        return ast;
    vector<Token> args = {tokens.begin()+3,tokens.end()-1};
    if(args.size()<2)
        parseError("SyntaxError","Invalid Syntax");
    size_t k = 0;
    bool found_default = false;//found any default argument
    string tname;
    while(k<args.size())
    {
    if((args[k].type== TokenType::KEYWORD_TOKEN && args[k].content=="var"))
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
        if(args[k].type==OP_TOKEN && args[k].content=="=")//default parameters
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
            vector<Token> default_expr = {args.begin()+k,args.begin()+j};
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
int k=0;
while(k<(int)tokens.size())
    {
        if(tokens[k].type!= TokenType::OP_TOKEN)
        {
            k+=1;
            continue;
        }
        if(tokens[k].content=="=")
        {
            Node* ast = NewNode(NodeType::assign,tokens[k].content);
            vector<Token> left = {tokens.begin(),tokens.begin()+k};
            if(left.size()==0)
                parseError("SyntaxError","Invalid Syntax");
            Node* n = NewNode(NodeType::line,to_string(tokens[k].ln));
            ast->childs.push_back(n);
            ast->childs.push_back(parseExpr(left));
            vector<Token> right = {tokens.begin()+k+1,tokens.end()};
            if(right.size()==0)
                parseError("SyntaxError","Invalid Syntax");
            if(right[0].type==KEYWORD_TOKEN && right[0].content=="yield")
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
        else if(tokens[k].content=="+=" || tokens[k].content=="-=" || tokens[k].content=="/=" || tokens[k].content=="*=" || tokens[k].content=="^=" || tokens[k].content=="%=" || tokens[k].content=="|=" || tokens[k].content=="&=" || tokens[k].content=="<<=" || tokens[k].content==">>=")
        {
            Node* ast = NewNode(NodeType::assign);
            vector<Token> left = {tokens.begin(),tokens.begin()+k};
            //line_num  = tokens[k].ln;
            if(left.size()==0 )
                parseError("SyntaxError","Invalid Syntax");
            Node* n = NewNode(NodeType::line,to_string(tokens[k].ln));
            ast->childs.push_back(n);
            ast->childs.push_back(parseExpr(left));
            Token o;
            o.type = OP_TOKEN;
            o.content+=tokens[k].content[0];
            left.push_back(o);
            vector<Token> right = {tokens.begin()+k+1,tokens.end()};
            if(right.size()==0)
                parseError("SyntaxError","Invalid Syntax");
            
            right.insert(right.begin(),left.begin(),left.end());
            ast->childs.push_back(parseExpr(right));
            return ast;
        }
        k+=1;
    }
    
    
    if(tokens[0].type==TokenType::KEYWORD_TOKEN && tokens[0].content=="try")
    {
        Node* line = NewNode(NodeType::line,to_string(tokens[0].ln));
        Node* ast = NewNode(NodeType::TRYCATCH);
        ast->childs.push_back(line);
        return ast;
    }
    if(tokens[0].type==TokenType::KEYWORD_TOKEN && tokens[0].content=="throw")
    {
    if(tokens.size()<2)
        parseError("SyntaxError","Invalid Syntax");
    vector<Token> expr = {tokens.begin()+1,tokens.end()};
    Node* line = NewNode(NodeType::line ,to_string(tokens[0].ln));
    Node* ast = NewNode(NodeType::THROW);
    ast->childs.push_back(line);
    ast->childs.push_back(parseExpr(expr));
    return ast;
    }
//Handle statements of the form
//expr.fun()
k = tokens.size()-1;
while(k>=0)
{
    if(tokens[k].type==TokenType::RParen_TOKEN)
    {
    int i = matchTokenRight(k,TokenType::LParen_TOKEN,tokens);
    if(i==-1)
        parseError("SyntaxError","Invalid Syntax");
    k = i;
    }
    else if(tokens[k].type== TokenType::END_LIST_TOKEN)
    {
        int i = matchTokenRight(k,TokenType::BEGIN_LIST_TOKEN,tokens);
        if(i==-1)
            parseError("SyntaxError","Invalid Syntax");
        k=i;
    }
    else if(tokens[k].type== TokenType::R_CURLY_BRACKET_TOKEN)
    {
    int i = matchTokenRight(k,TokenType::L_CURLY_BRACKET_TOKEN,tokens);
    if(i==-1)
        parseError("SyntaxError","Invalid Syntax");
    k=i;
    }
    else if(tokens[k].type== TokenType::OP_TOKEN && (tokens[k].content=="."))
    {
    vector<Token> lhs = {tokens.begin(),tokens.begin()+k};
    vector<Token> rhs = {tokens.begin()+k+1,tokens.end()};
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
        deleteAST(ast);
        break;
    }
    // if(L->type!=NodeType::memb && L->type!=NodeType::ID && L->type!=NodeType::index && L->type!=NodeType::call)
    //   {
    //    deleteAST(ast);
    //    break;
    //   }
    return ast;
    }
    k-=1;
}
if(REPL_MODE)
{
    vector<Token> dummyStmt;
    Token i;
    i.content = "println";
    i.ln = tokens[0].ln;
    i.type = ID_TOKEN;
    dummyStmt.push_back(i);
    i.content = "(";
    i.type = LParen_TOKEN;
    dummyStmt.push_back(i);
    dummyStmt.insert(dummyStmt.end(),tokens.begin(),tokens.end());
    i.content = ")";
    i.type = RParen_TOKEN;
    dummyStmt.push_back(i);
    return parseExpr(dummyStmt);
}
parseError("SyntaxError","Unknown statement");
return nullptr;//to avoid compiler warning otherwise the parseError function exits after printing error message

}

Node* Parser::parse(const vector<Token>& tokens)
{
    size_t k = 0;
    Node* ast = nullptr;
    Token ElseTok,elif,newlinetok,bgscope,endscope;
    int start = 0;
    Node* e = nullptr;//e points to the lowest rightmost node of ast
    Node* Final = nullptr;
    line_num = 1;
    bool a,b,c;
    size_t len = tokens.size();
    while(k<len)
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
            //for ifelse and loop statements, we pop the curly bracket from line
            //for multiline expressions we don't
            //multiline expressions begin by adding '{' or '(' or '[' at the end
            //of a line that is not a conditional statement or loop stmt
            if
            (
                (a = line.back().type == TokenType::L_CURLY_BRACKET_TOKEN) ||
                (b = line.back().type == TokenType::LParen_TOKEN) ||
                (c = line.back().type == TokenType::BEGIN_LIST_TOKEN)
            )
            {
            if(line.size() == 1) ;//
            else
            {
                if( line[0].type != TokenType::KEYWORD_TOKEN ||
                    (line[0].type == TokenType::KEYWORD_TOKEN &&
                        (line[0].content == "var" || line[0].content=="return" || line[0].content=="yield")
                    )
                )
                {
                int idx = (int)k -1;
                int rp = matchToken(idx,TokenType::R_CURLY_BRACKET_TOKEN,tokens);
                if(a)
                    rp = matchToken(idx,TokenType::R_CURLY_BRACKET_TOKEN,tokens);
                else if(b)
                    rp = matchToken(idx,TokenType::RParen_TOKEN,tokens);
                else if(c)
                    rp = matchToken(idx,TokenType::END_LIST_TOKEN,tokens);
                if(rp == -1)
                    parseError("SyntaxError","'"+line.back().content+"' at the end of line is unmatched.");
                //rp currently includes ending bracket on some line
                //that whole line should be included
                while(tokens[rp].type!=NEWLINE_TOKEN)
                    rp++;
                size_t i = 0;
                for(i=k;i<(size_t)rp ;i++)
                {
                    if(tokens[i].type != TokenType::NEWLINE_TOKEN)
                    line.push_back(tokens[i]);
                }
                k = i;//tokens before ith idx are handled

                }
                else if(a)//pop curly bracket only
                line.pop_back();
            }
            }
            if(line.size()==0)
            {
                start+=1;
                k+=1;
                continue;
            }


            ast = parseStmt(line);


            if(ast->type==NodeType::IF)
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
                {
                    i = matchToken(j,TokenType::R_CURLY_BRACKET_TOKEN,tokens);
                    if(i == -1)
                    {
                      line_num = tokens[j].ln;
                      parseError("SyntaxError","Error expected '}' to match this.");
                    }
                }
                else
                {
                    j = findTokenConsecutive(newlinetok,start+line.size(),tokens);
                    i = findToken(newlinetok,j+1,tokens);
                    line_num = line[0].ln;
                    if(i==-1 || (j<(int)tokens.size()-1 && tokens[j+1].type==EOP_TOKEN))
                    parseError("SyntaxError","Error expected code block or line after if!");
                }
                vector<Token> block = {tokens.begin()+j+1,tokens.begin()+i};//if block
                if(block.size()==1 && block[0].type==KEYWORD_TOKEN && (block[0].content=="endfor" || tokens[0].content=="endtry" || tokens[0].content=="endcatch" || block[0].content=="endwhile" || block[0].content=="endnm" || block[0].content=="endclass" || block[0].content=="endfunc" || block[0].content=="endelse" || block[0].content=="endelif" ||  block[0].content=="endif"))
                {
                    parseError("SyntaxError","Error use brackets {} after if!");
                }
                stripNewlines(block);
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
                if(p==-1 || p!=j+1)
                {

                    line_num = tokens[j].ln;
                    parseError("SyntaxError","Error expected '(' after else if");
                }
                int l = matchToken(p,TokenType::RParen_TOKEN,tokens);

                vector<Token> condition = {tokens.begin()+p+1,tokens.begin()+l};
                elifConditions.push_back(condition);
                j = findTokenConsecutive(bgscope,l+1,tokens);
                if(j==-1)
                {
                    j = findTokenConsecutive(newlinetok,l+1,tokens);
                    i = findToken(newlinetok,j+1,tokens);
                    line_num = tokens[p].ln;
                    j-=1;
                    if(i==-1 || (j+1<(int)tokens.size()-1 && tokens[j+2].type==EOP_TOKEN))
                        parseError("SyntaxError","Error expected code block or line after else if");
                }
                else
                    i = matchToken(j,TokenType::R_CURLY_BRACKET_TOKEN,tokens);
                vector<Token> elifBlock = {tokens.begin()+j+1+1,tokens.begin()+i};
                if(elifBlock.size()==1 && elifBlock[0].type==KEYWORD_TOKEN && (elifBlock[0].content=="endfor" || elifBlock[0].content=="endwhile" || elifBlock[0].content=="endnm" || elifBlock[0].content=="endclass" || elifBlock[0].content=="endfunc" || elifBlock[0].content=="endelse" || elifBlock[0].content=="endelif" ||  elifBlock[0].content=="endif"))
                {
                    parseError("SyntaxError","Error use brackets {} after else if!");
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
                        if(i==-1 || (j<(int)tokens.size()-1 && tokens[j+1].type==EOP_TOKEN))
                        parseError("SyntaxError","Error expected code block or line after else");
                    }
                    else
                    i = matchToken(j,TokenType::R_CURLY_BRACKET_TOKEN,tokens);
                }
                if(foundelif && foundElse)
                {
                ast->type = NodeType::IFELIFELSE;
                vector<Token> elseBlock = {tokens.begin()+j+1,tokens.begin()+i};
                if(elseBlock.size()==1 && elseBlock[0].type==KEYWORD_TOKEN && (elseBlock[0].content=="endfor" || elseBlock[0].content=="endwhile" || elseBlock[0].content=="endnm" || elseBlock[0].content=="endclass" || elseBlock[0].content=="endfunc" || elseBlock[0].content=="endelse" || elseBlock[0].content=="endelif" ||  elseBlock[0].content=="endif"))
                {
                    parseError("SyntaxError","Error use brackets {} after else!");
                }
                stripNewlines(elseBlock);
                for(int a=0;a<(int)elifConditions.size();a++)
                {
                    line_num = elifConditions[a][0].ln;   
                    ast->childs[1]->childs.push_back(parseExpr(elifConditions[a]));
                }

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
                bool ctxCopy = inif;
                inif = true;
                ast->childs.push_back(parse(block));
                inif = ctxCopy;
                t.content = "endelif";
                for(int a=0;a<(int)elifBlocks.size();a++)
                {
                    vector<Token> elifBlock = elifBlocks[a];
                    stripNewlines(elifBlock);
                    if(elifBlock.size()!=0)
                        elifBlock.push_back(newlinetok);
                    elifBlock.push_back(t);
                    elifBlock.push_back(newlinetok);
                    bool ctxCopy = inelif;
                    inelif = true;
                    Node* n = parse(elifBlock);
                    inelif = ctxCopy;
                    ast->childs.push_back(n);
                }
                ctxCopy = inelse;
                inelse = true;
                ast->childs.push_back(parse(elseBlock));
                inelse = ctxCopy;
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
                ast->type = NodeType::IFELIF;
                for(int a=0;a<(int)elifConditions.size();a++)
                {
                    line_num = elifConditions[a][0].ln;
                    ast->childs[1]->childs.push_back(parseExpr(elifConditions[a]));
                }
                Token t;
                t.content = "endif";
                t.type=KEYWORD_TOKEN;
                if(block.size()!=0)
                    block.push_back(newlinetok);
                block.push_back(t);
                block.push_back(newlinetok);
                bool ctxCopy = inif;
                inif = true;
                ast->childs.push_back(parse(block));
                inif = ctxCopy;
                t.content = "endelif";
                for(int a=0;a<(int)elifBlocks.size();a++)
                {
                    vector<Token> elifBlock = elifBlocks[a];
                    stripNewlines(elifBlock);
                    if(elifBlock.size()!=0)
                        elifBlock.push_back(newlinetok);
                    elifBlock.push_back(t);
                    elifBlock.push_back(newlinetok);
                    bool ctxCopy = inelif;
                    inelif = true;
                    Node* n = parse(elifBlock);
                    inelif = ctxCopy;
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
                ast->type = NodeType::IFELSE;
                vector<Token> elseBlock = {tokens.begin()+j+1,tokens.begin()+i};
                stripNewlines(elseBlock);
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
                bool ctxCopy = inif;
                inif = true;
                ast->childs.push_back(parse(block));
                inif = ctxCopy;
                ctxCopy = inelse;
                inelse = true;
                ast->childs.push_back(parse(elseBlock));
                inelse = ctxCopy;
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
                    ast->type = NodeType::IF;
                    Token t;
                    t.content = "endif";
                    t.type = KEYWORD_TOKEN;
                    if(block.size()!=0)
                        block.push_back(newlinetok);
                    block.push_back(t);
                    block.push_back(newlinetok);
                    bool ctxCopy = inif;
                    inif = true;
                    ast->childs.push_back(parse(block));
                    inif = ctxCopy;
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
            else if(ast->type==NodeType::FUNC)
            {
                bgscope.content = "{";
                bgscope.type=L_CURLY_BRACKET_TOKEN;
                int j = findTokenConsecutive(bgscope,start+line.size(),tokens);
                int i = -1;
                if(j==-1)
                {
                    newlinetok.content = "\n";
                    newlinetok.type=NEWLINE_TOKEN;
                    j = findTokenConsecutive(newlinetok,start+line.size(),tokens);
                    if(j==-1)
                        parseError("SyntaxError","Expected code block after function definition!");
                    i = findToken(newlinetok,j+1,tokens);
                    if(i==-1)
                        parseError("SyntaxError","Expected code block after function definition!");
                }
                else
                    i = matchToken(j,TokenType::R_CURLY_BRACKET_TOKEN,tokens);
                if(i==-1)
                    parseError("SyntaxError","Error missing '}' to end function definition.");
                if(j==-1 || (j<(int)tokens.size()-1 && tokens[j+1].type==EOP_TOKEN))
                    parseError("SyntaxError","Error expected code block or line after function definition!");

                vector<Token> block = {tokens.begin()+j+1,tokens.begin()+i};
                if(block.size()==1 && block[0].type==KEYWORD_TOKEN && (block[0].content=="endfor" || tokens[0].content=="endtry" || tokens[0].content=="endcatch" || block[0].content=="endwhile" || block[0].content=="endnm" || block[0].content=="endclass" || block[0].content=="endfunc" || block[0].content=="endelse" || block[0].content=="endelif" || block[0].content=="endif"))
                {
                    parseError("SyntaxError","Error expected code block or line after function definition!");
                }
                if(infunc)
                    parseError("SyntaxError","Nested functions not allowed!");
                stripNewlines(block);
                newlinetok.type = NEWLINE_TOKEN;
                newlinetok.content = "\n";
                if(block.size()!=0)
                    block.push_back(newlinetok);
                Token t;
                t.type=KEYWORD_TOKEN;
                t.content = "endfunc";
                block.push_back(t);
                block.push_back(newlinetok);
                if(!isValidCtxForFunc())
                    parseError("SyntaxError","Local functions not allowed!");
                foundYield = false;
                string aux = currSym;
                if(!inclass)
                {
                    currSym = ast->childs[1]->val;
                    refGraph->emplace(ast->childs[1]->val,vector<string>{});
                }
                infunc = true;
                ast->childs.push_back(parse(block));
                infunc = false;
                if(!inclass)
                    currSym = aux;

                if(foundYield)
                    ast->type = NodeType::CORO;

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
            else if(ast->type==NodeType::CLASS || ast->type==NodeType::EXTCLASS)
            {
                bgscope.content = "{";
                bgscope.type=L_CURLY_BRACKET_TOKEN;
                int j = findTokenConsecutive(bgscope,start+line.size(),tokens);
                int i = -1;
                if(j==-1)
                {
                    parseError("SyntaxError","Error expected brackets {} after class definition!");
                }
                else
                    i = matchToken(j,TokenType::R_CURLY_BRACKET_TOKEN,tokens);
                if(!atGlobalLevel())
                    parseError("SyntaxError","Class declartions must be at global scope or in a namespace!");
                vector<Token> block = {tokens.begin()+j+1,tokens.begin()+i};
                stripNewlines(block);
                newlinetok.type = NEWLINE_TOKEN;
                newlinetok.content = "\n";
                if(block.size()!=0)
                    block.push_back(newlinetok);
                Token t;
                t.type=KEYWORD_TOKEN;
                t.content = "endclass";
                block.push_back(t);
                block.push_back(newlinetok);
                inclass = true;
                string aux = currSym;
                currSym = ast->childs[1]->val;
                refGraph->emplace(ast->childs[1]->val,vector<string>{});
                if(ast->val=="class")
                    ast->childs.push_back(parse(block));
                else
                    ast->childs.insert(ast->childs.begin()+2,parse(block));
                //backtrack
                //important: change if planning to support nested classes
                inclass = false;
                currSym = aux;
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
            else if(ast->type==NodeType::NAMESPACE)
            {
                bgscope.content = "{";
                bgscope.type=L_CURLY_BRACKET_TOKEN;
                int j = findTokenConsecutive(bgscope,start+line.size(),tokens);
                int i = -1;
                if(j==-1)
                {
                    line_num = line[0].ln;
                    parseError("SyntaxError","Error expected {} after namespace keyword");
                }
                else
                    i = matchToken(j,TokenType::R_CURLY_BRACKET_TOKEN,tokens);
                if(!atGlobalLevel())
                    parseError("SyntaxError","Namespace declaration must be at global scope or within another namespace!");
                vector<Token> block = {tokens.begin()+j+1,tokens.begin()+i};
                stripNewlines(block);
                newlinetok.type = NEWLINE_TOKEN;
                newlinetok.content = "\n";
                if(block.size()!=0)
                    block.push_back(newlinetok);
                Token t;
                t.type=KEYWORD_TOKEN;
                t.content = "endnm";//end namespace
                block.push_back(t);
                block.push_back(newlinetok);
                string prefix;
                for(auto e: prefixes)
                    prefix+=e;
                prefix += ast->childs[1]->val+"::";
                prefixes.push_back(prefix);//prefix each identifier in namespace block
                //with "namespaceName::"
                ast->childs.push_back(parse(block));
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
            else if(ast->type==NodeType::WHILE || ast->type==NodeType::DOWHILE || ast->type==NodeType::FOR || ast->type == NodeType::DFOR || ast->type==NodeType::FOREACH)//loops
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
                    if(i==-1 || (j<(int)tokens.size()-1 && tokens[j+1].type==EOP_TOKEN))
                        parseError("SyntaxError","Error expected a code block or line after "+ast->val);
                }
                else
                    i = matchToken(j,TokenType::R_CURLY_BRACKET_TOKEN,tokens);
                vector<Token> block = {tokens.begin()+j+1,tokens.begin()+i};
                if(block.size()==1 && block[0].type==KEYWORD_TOKEN && (block[0].content=="endfor" || tokens[0].content=="endtry" || tokens[0].content=="endcatch" || block[0].content=="endwhile" || block[0].content=="endnm" || block[0].content=="endclass" || block[0].content=="endfunc" || block[0].content=="endelse" || block[0].content=="endelif" ||  block[0].content=="endif"))
                {
                    parseError("SyntaxError","Error use brackets {} after "+ast->val);
                }
                stripNewlines(block);
                Token t;
                if(ast->val=="while" || ast->val=="dowhile")
                    t.content = "endwhile";
                else
                    t.content = "endfor";
                t.type = KEYWORD_TOKEN;
                Token newlinetok;
                newlinetok.type = NEWLINE_TOKEN;
                newlinetok.content = "\n";
                if(block.size()!=0)
                    block.push_back(newlinetok);
                block.push_back(t);
                block.push_back(newlinetok);
                bool b = inloop;
                inloop = true;
                ast->childs.push_back(parse(block));//inwhile = true
                inloop = b;
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
            else if(ast->type==NodeType::TRYCATCH)//try catch
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
                    if(i==-1 || (i<(int)tokens.size()-1 && tokens[i+1].type==EOP_TOKEN))
                    parseError("SyntaxError","Error expected a code block or line after try!");
                }
                else
                    i = matchToken(j,TokenType::R_CURLY_BRACKET_TOKEN,tokens);
                vector<Token> block = {tokens.begin()+j+1,tokens.begin()+i};
                if(block.size()==1 && block[0].type==KEYWORD_TOKEN && (block[0].content=="endfor" || block[0].content=="endwhile" || block[0].content=="endnm" || block[0].content=="endclass" || block[0].content=="endfunc" || tokens[0].content=="endtry" || tokens[0].content=="endcatch" || block[0].content=="endelse" || block[0].content=="endelif" ||  block[0].content=="endif"))
                {
                    parseError("SyntaxError","Error use brackets {} after "+ast->val);
                }
                stripNewlines(block);
                Token CATCH_TOK;
                CATCH_TOK.type = TokenType::KEYWORD_TOKEN;
                CATCH_TOK.content = "catch";
                string catchId ;
                j = findTokenConsecutive(CATCH_TOK,i+1,tokens);
                if(j!=-1)
                {
                    line_num = tokens[j].ln;
                    if(j+4>=(int)tokens.size())
                    {
                    parseError("SyntaxError","Invalid Syntax");
                    }
                    j+=1;
                    if(tokens[j].type!=TokenType::LParen_TOKEN)
                        parseError("SyntaxError","Invalid Syntax");
                    j+=1;
                    if(tokens[j].type!=TokenType::ID_TOKEN)
                        parseError("SyntaxError","Invalid Syntax");
                    catchId = tokens[j].content;
                    j+=1;
                    if(tokens[j].type!=TokenType::RParen_TOKEN)
                        parseError("SyntaxError","Invalid Syntax");
                    j+=1;
                    j = findTokenConsecutive(bgscope,j,tokens);
                    int e = j;
                    if(j==-1)
                    {
                        j = findTokenConsecutive(newlinetok,e+1,tokens);
                        i = findToken(newlinetok,j+1,tokens);
                        if(i==-1 || (i<(int)tokens.size()-1 && tokens[i+1].type==EOP_TOKEN))
                            parseError("SyntaxError","Error expected code block or line after catch");
                    }
                    else
                        i = matchToken(j,TokenType::R_CURLY_BRACKET_TOKEN,tokens);
                }
                else
                {
                    parseError("SyntaxError","Error expected catch block after try{}!");
                }
                vector<Token> catchBlock = {tokens.begin()+j+1,tokens.begin()+i};
                stripNewlines(catchBlock);
                Token t;
                t.content = "endtry";
                t.type = KEYWORD_TOKEN;
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
                ast->childs.push_back(NewNode(NodeType::ID,catchId));
                bool ctxCopy = intry;
                intry = true;
                ast->childs.push_back(parse(block));
                intry = ctxCopy;
                ctxCopy = incatch;
                incatch = true;
                ast->childs.push_back(parse(catchBlock));
                incatch = ctxCopy;
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
            else if(ast->type==NodeType::BREAK || ast->type==NodeType::CONTINUE)
            {
            if(!inloop)
            {
                parseError("SyntaxError","Error use of break or continue not allowed outside loop!");
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
            else if(ast->type==NodeType::RETURN_NODE)
            {
            if(!infunc)
            {
                parseError("SyntaxError","Error use of return statement outside functon!");
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
            else if(ast->type==NodeType::import)
            {
            string str = ast->childs[1]->val;
            if(ast->childs[1]->type!=NodeType::ID)
            {
                Node* A;
                if(std::find(files->begin(),files->end(),str)!=files->end())
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
                    files->push_back(str);
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
                    sources->push_back(src);
                    Lexer lex;
                    vector<Token> tokens = lex.generateTokens(filename,src);
                    bool empty_file = false;
                    if(tokens.size() == 1 && tokens[0].type==EOP_TOKEN)
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
                        subast = parse(tokens);
                    A->childs.push_back(subast);
                    filename = F;
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