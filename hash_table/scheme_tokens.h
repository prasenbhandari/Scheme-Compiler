
/* scheme_tokens.h */
#ifndef SCHEME_TOKENS_H
#define SCHEME_TOKENS_H

typedef enum {
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_KEYWORD,
    TOKEN_LITERAL,
    TOKEN_DEC,
    TOKEN_REAL,
    TOKEN_STR_LITERAL,
    TOKEN_ID,
    TOKEN_IF,
    TOKEN_DEFINE,
    TOKEN_DISPLAY,
    TOKEN_LAMBDA,
    TOKEN_LET,
    TOKEN_CDR,
    TOKEN_CAR,
    TOKEN_COND
} token_type;

#endif
