#ifndef TOKEN_H
#define TOKEN_H

#include <stdint.h>

typedef enum {
    // Special forms (keywords) - these control evaluation and cannot be redefined
    TOKEN_AND, TOKEN_BEGIN, TOKEN_CASE, TOKEN_COND, TOKEN_DEFINE,
    TOKEN_DELAY, TOKEN_DO, TOKEN_ELSE, TOKEN_IF, TOKEN_LAMBDA,
    TOKEN_LET, TOKEN_LET_STAR, TOKEN_LETREC, TOKEN_LETREC_STAR,
    TOKEN_OR, TOKEN_QUASIQUOTE, TOKEN_QUOTE, TOKEN_SET, TOKEN_UNQUOTE,
    TOKEN_UNLESS, TOKEN_WHEN,
    
    // Other token types
    TOKEN_DEC, TOKEN_REAL, TOKEN_IDENTIFIER, TOKEN_LPAREN, TOKEN_RPAREN,
    TOKEN_STR_LITERAL, TOKEN_DOT, TOKEN_QUOTE_MARK, TOKEN_BACKQUOTE, TOKEN_COMMA, TOKEN_TRUE, TOKEN_FALSE,
    
    // Symbol tokens
    TOKEN_SYMBOL // quoted symbol
} TokenType;

typedef struct {
    union {
        char* lexeme;
        int64_t int_value;
        double real_value;
    };   
    TokenType type;
    uint32_t line;      // Source line number
    uint32_t column;    // Source column number
} Token;

Token* create_token(const char* lexeme, TokenType type);
void free_token(Token* t);
const char* token_type_to_string(TokenType type);
TokenType check_keyword(const char* lexeme);

#endif // TOKEN_H
