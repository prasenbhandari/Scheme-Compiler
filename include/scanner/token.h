#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    // Special forms (keywords) - these control evaluation and cannot be redefined
    TOKEN_AND, TOKEN_BEGIN, TOKEN_CASE, TOKEN_COND, TOKEN_DEFINE,
    TOKEN_DELAY, TOKEN_DO, TOKEN_IF, TOKEN_LAMBDA,
    TOKEN_LET, TOKEN_LET_STAR, TOKEN_LETREC, TOKEN_LETREC_STAR,
    TOKEN_OR, TOKEN_QUASIQUOTE, TOKEN_QUOTE, TOKEN_SET, TOKEN_UNQUOTE,
    TOKEN_UNLESS, TOKEN_WHEN,
    
    // Other token types
    TOKEN_DEC, TOKEN_REAL, TOKEN_IDENTIFIER, TOKEN_LPAREN, TOKEN_RPAREN,
    TOKEN_STR_LITERAL, TOKEN_DOT, TOKEN_QUOTE_MARK, TOKEN_BACKQUOTE, TOKEN_COMMA,
    
    // Symbol tokens
    TOKEN_SYMBOL // quoted symbol
} token_type;

typedef struct {
    union {
        char* lexeme;
        int int_value;
        double real_value;
    };   
    token_type type;
} token;

token* create_token(const char* lexeme, token_type type);
void free_token(token* t);
const char* token_type_to_string(token_type type);
token_type check_keyword(const char* lexeme);

#endif // TOKEN_H
