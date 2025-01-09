#include "token.h"

token make_token(TokenType t,const char* content,size_t ln)
{
    token tok;
    tok.type = t;
    tok.content = (char*)content;
    tok.ln = ln;
    return tok;
}