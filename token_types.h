#ifndef TOKEN_TYPES_H
#define TOKEN_TYPES_H

typedef enum {
    // Special forms and syntax keywords (alphabetically ordered)
    TOKEN_AND,
    TOKEN_BEGIN,
    TOKEN_CASE,
    TOKEN_COND,
    TOKEN_DEFINE,
    TOKEN_DELAY,
    TOKEN_DO,
    TOKEN_FORCE,
    TOKEN_IF,
    TOKEN_LAMBDA,
    TOKEN_LET,
    TOKEN_LET_STAR,    // let*
    TOKEN_LETREC,
    TOKEN_LETREC_STAR, // letrec*
    TOKEN_OR,
    TOKEN_QUASIQUOTE,
    TOKEN_QUOTE,
    TOKEN_SET,         // set!
    TOKEN_UNQUOTE,
    TOKEN_UNLESS,
    TOKEN_WHEN,
    
    // Common procedures (alphabetically ordered)
    TOKEN_APPEND,
    TOKEN_APPLY,
    TOKEN_CAR,
    TOKEN_CDR,
    TOKEN_CONS,
    TOKEN_DISPLAY,
    TOKEN_EQ_P,        // eq?
    TOKEN_EQUAL_P,     // equal?
    TOKEN_LENGTH,
    TOKEN_LIST,
    TOKEN_MAP,
    TOKEN_NOT,
    TOKEN_NULL_P,      // null?
    TOKEN_NUMBER_P,    // number?
    TOKEN_PAIR_P,      // pair?
    TOKEN_PROCEDURE_P, // procedure?
    TOKEN_REVERSE,
    TOKEN_STRING_P,    // string?
    TOKEN_SYMBOL_P,    // symbol?
    
    // Other token types
    TOKEN_DEC,
    TOKEN_EQ,          // =
    TOKEN_IDENTIFIER,  // renamed from TOKEN_ID
    TOKEN_LPAREN,
    TOKEN_REAL,
    TOKEN_RPAREN,
    TOKEN_STR_LITERAL,
} token_type;

typedef struct {
    union {
        char* lexeme;
        int int_value;
        double real_value;
    };
    token_type type;
} token;

#endif // TOKEN_TYPES_H
