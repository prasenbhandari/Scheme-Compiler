#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    // Special forms and syntax keywords
    TOKEN_AND, TOKEN_BEGIN, TOKEN_CASE, TOKEN_COND, TOKEN_DEFINE,
    TOKEN_DELAY, TOKEN_DO, TOKEN_FORCE, TOKEN_IF, TOKEN_LAMBDA,
    TOKEN_LET, TOKEN_LET_STAR, TOKEN_LETREC, TOKEN_LETREC_STAR,
    TOKEN_QUASIQUOTE, TOKEN_QUOTE, TOKEN_SET, TOKEN_UNQUOTE,
    TOKEN_UNLESS, TOKEN_WHEN,
    
    // Common procedures
    TOKEN_APPEND, TOKEN_APPLY, TOKEN_CAR, TOKEN_CDR, TOKEN_CONS,
    TOKEN_DISPLAY, TOKEN_EQ_P, TOKEN_EQUAL_P, TOKEN_LENGTH,
    TOKEN_LIST, TOKEN_MAP, TOKEN_NOT, TOKEN_NULL_P, TOKEN_NUMBER_P,
    TOKEN_PAIR_P, TOKEN_PROCEDURE_P, TOKEN_REVERSE, TOKEN_STRING_P,
    TOKEN_SYMBOL_P, // string?
    
    // Arithmetic operators
    TOKEN_PLUS,        // +
    TOKEN_MINUS,       // -
    TOKEN_MULTIPLY,    // *
    TOKEN_DIVIDE,      // /
    TOKEN_MOD,         // modulo
    TOKEN_ABS,         // abs
    TOKEN_MAX,         // max
    TOKEN_MIN,         // min
    TOKEN_SQRT,        // sqrt
    TOKEN_EXPT,        // expt (exponent)
    
    // Comparison operators
    TOKEN_EQ,          // = (numeric equality)
    TOKEN_LT,          // <
    TOKEN_GT,          // >
    TOKEN_LTE,         // <=
    TOKEN_GTE,         // >=
    TOKEN_NEQ,         // not=
    
    // Logical operators
    TOKEN_NOT_OP,      // not
    TOKEN_AND_OP,      // and
    TOKEN_OR_OP,       // or
    
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
