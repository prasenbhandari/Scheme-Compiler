#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 4096
#define MAX_LEXEME_SIZE 256
#define SYMBOL_TABLE_SIZE 211  // A prime number for better hash distribution

typedef enum {
    // Special forms and syntax keywords
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
    
    // Common procedures
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
    TOKEN_SYMBOL,      // symbol
    
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
    TOKEN_EXPT,        // expt (exponentiation)
    
    // Comparison operators
    TOKEN_EQ,          // =  (numeric equality)
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
    TOKEN_DEC,         // Decimal number
    TOKEN_REAL,        // Real number
    TOKEN_IDENTIFIER,  // Identifier
    TOKEN_LPAREN,      // (
    TOKEN_RPAREN,      // )
    TOKEN_STR_LITERAL, // String literal
    TOKEN_DOT,         // .
    TOKEN_QUOTE_MARK,  // '
    TOKEN_BACKQUOTE,   // `
    TOKEN_COMMA        // ,
} token_type;

// Forward declarations
static void report_error(const char* message);

typedef struct {
    char* lexeme;
    token_type type;
} token;

typedef struct symbol_entry {
    char* lexeme;
    token_type type;
    struct symbol_entry* next;
} symbol_entry;

typedef struct {
    union {
        char* lexeme;
        int int_value;
        double real_value;
    };
    token_type type;
} token;

typedef struct symbol_entry {
    char* lexeme;
    token_type type;
    struct symbol_entry* next;
} symbol_entry;

static symbol_entry* symbol_table[SYMBOL_TABLE_SIZE] = {NULL};

static unsigned int hash(const char* str) {
    unsigned int hash = 0;
    while (*str) {
        hash = (hash * 31) + *str++;
    }
    return hash % SYMBOL_TABLE_SIZE;
}

static symbol_entry* lookup_symbol(const char* lexeme) {
    unsigned int index = hash(lexeme);
    symbol_entry* entry = symbol_table[index];
    
    while (entry != NULL) {
        if (strcmp(entry->lexeme, lexeme) == 0) {
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}

static void insert_symbol(const char* lexeme, token_type type) {
    unsigned int index = hash(lexeme);
    symbol_entry* entry = malloc(sizeof(symbol_entry));
    if (!entry) {
        report_error("Memory allocation failed for symbol table");
        return;
    }
    
    entry->lexeme = strdup(lexeme);
    entry->type = type;
    entry->next = symbol_table[index];
    symbol_table[index] = entry;
}

static void cleanup_symbol_table(void) {
    for (int i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        symbol_entry* entry = symbol_table[i];
        while (entry != NULL) {
            symbol_entry* next = entry->next;
            free(entry->lexeme);
            free(entry);
            entry = next;
        }
        symbol_table[i] = NULL;
    }
}

// Error handling
typedef struct {
    int line;
    int column;
    const char* message;
} scanner_error;

static scanner_error current_error = {0, 0, NULL};
static int current_line = 1;
static int current_column = 0;

static void report_error(const char* message) {
    current_error.line = current_line;
    current_error.column = current_column;
    current_error.message = message;
    fprintf(stderr, "Error at line %d, column %d: %s\n", 
            current_line, current_column, message);
}

// Double buffering
static char buffers[2][BUFFER_SIZE];
static int current_buffer = 0;
static int buffer_pointer = 0;
static int buffer_end[2] = {0, 0};
static int eof_reached = 0;

static int fill_buffer(FILE* file, int buffer_num) {
    size_t bytes_read = fread(buffers[buffer_num], 1, BUFFER_SIZE - 1, file);
    if (bytes_read < BUFFER_SIZE - 1) {
        if (ferror(file)) {
            report_error("Error reading from file");
            return -1;
        }
        eof_reached = 1;
    }
    buffers[buffer_num][bytes_read] = '\0';
    buffer_end[buffer_num] = bytes_read;
    return bytes_read;
}

static int get_next_char(FILE* file) {
    // Check if we need to switch buffers
    if (buffer_pointer >= buffer_end[current_buffer]) {
        if (eof_reached) {
            return EOF;
        }
        
        // Switch to other buffer and fill it
        current_buffer = 1 - current_buffer;
        buffer_pointer = 0;
        
        if (fill_buffer(file, current_buffer) <= 0) {
            return EOF;
        }
    }
    
    int c = buffers[current_buffer][buffer_pointer++];
    if (c == '\n') {
        current_line++;
        current_column = 0;
    } else {
        current_column++;
    }
    return c;
}

static void unget_char(void) {
    if (buffer_pointer > 0) {
        buffer_pointer--;
        if (buffers[current_buffer][buffer_pointer] == '\n') {
            current_line--;
            // Column position after ungetting a newline is approximate
            current_column = 0;
        } else {
            current_column--;
        }
    }
}

token* create_token(const char* lexeme, token_type type) {
    token* temp = malloc(sizeof(token));
    if (!temp) {
        fprintf(stderr, "Memory allocation failed for token\n");
        exit(1);
    }

    temp->type = type;

    if(type == TOKEN_DEC) {
        temp->int_value = atoi(lexeme);
    } else if(type == TOKEN_REAL) {
        temp->real_value = atof(lexeme);
    } else {
        temp->lexeme = strdup(lexeme);
        if (!temp->lexeme) {
            fprintf(stderr, "Memory allocation failed for lexeme\n");
            free(temp);
            exit(1);
        }
    }
    return temp;
}

// DFA-based keyword and operator recognition
token_type check_keyword(const char* lexeme) {
    switch(lexeme[0]) {
        case '+': return TOKEN_PLUS;
        case '-': return TOKEN_MINUS;
        case '*': return TOKEN_MULTIPLY;
        case '/': return TOKEN_DIVIDE;
        case '<':
            if (strcmp(lexeme, "<=") == 0) return TOKEN_LTE;
            return TOKEN_LT;
        case '>':
            if (strcmp(lexeme, ">=") == 0) return TOKEN_GTE;
            return TOKEN_GT;
        case '=': return TOKEN_EQ;
        case '.': return TOKEN_DOT;
        case '\'': return TOKEN_QUOTE_MARK;
        case '`': return TOKEN_BACKQUOTE;
        case ',': return TOKEN_COMMA;
        
        case 'a':
            if (strcmp(lexeme, "and") == 0) return TOKEN_AND;
            if (strcmp(lexeme, "append") == 0) return TOKEN_APPEND;
            if (strcmp(lexeme, "apply") == 0) return TOKEN_APPLY;
            if (strcmp(lexeme, "abs") == 0) return TOKEN_ABS;
            break;
            
        case 'b':
            if (strcmp(lexeme, "begin") == 0) return TOKEN_BEGIN;
            break;
            
        case 'c':
            if (strcmp(lexeme, "case") == 0) return TOKEN_CASE;
            if (strcmp(lexeme, "car") == 0) return TOKEN_CAR;
            if (strcmp(lexeme, "cdr") == 0) return TOKEN_CDR;
            if (strcmp(lexeme, "cond") == 0) return TOKEN_COND;
            if (strcmp(lexeme, "cons") == 0) return TOKEN_CONS;
            break;
            
        case 'd':
            if (strcmp(lexeme, "define") == 0) return TOKEN_DEFINE;
            if (strcmp(lexeme, "delay") == 0) return TOKEN_DELAY;
            if (strcmp(lexeme, "display") == 0) return TOKEN_DISPLAY;
            if (strcmp(lexeme, "do") == 0) return TOKEN_DO;
            break;
            
        case 'e':
            if (strcmp(lexeme, "eq?") == 0) return TOKEN_EQ_P;
            if (strcmp(lexeme, "equal?") == 0) return TOKEN_EQUAL_P;
            if (strcmp(lexeme, "expt") == 0) return TOKEN_EXPT;
            break;
            
        case 'f':
            if (strcmp(lexeme, "force") == 0) return TOKEN_FORCE;
            break;
            
        case 'i':
            if (strcmp(lexeme, "if") == 0) return TOKEN_IF;
            break;
            
        case 'l':
            if (strcmp(lexeme, "lambda") == 0) return TOKEN_LAMBDA;
            if (strcmp(lexeme, "length") == 0) return TOKEN_LENGTH;
            if (strcmp(lexeme, "let") == 0) return TOKEN_LET;
            if (strcmp(lexeme, "let*") == 0) return TOKEN_LET_STAR;
            if (strcmp(lexeme, "letrec") == 0) return TOKEN_LETREC;
            if (strcmp(lexeme, "letrec*") == 0) return TOKEN_LETREC_STAR;
            if (strcmp(lexeme, "list") == 0) return TOKEN_LIST;
            break;
            
        case 'm':
            if (strcmp(lexeme, "map") == 0) return TOKEN_MAP;
            if (strcmp(lexeme, "max") == 0) return TOKEN_MAX;
            if (strcmp(lexeme, "min") == 0) return TOKEN_MIN;
            if (strcmp(lexeme, "modulo") == 0) return TOKEN_MOD;
            break;
            
        case 'n':
            if (strcmp(lexeme, "not") == 0) return TOKEN_NOT_OP;
            if (strcmp(lexeme, "not=") == 0) return TOKEN_NEQ;
            if (strcmp(lexeme, "null?") == 0) return TOKEN_NULL_P;
            if (strcmp(lexeme, "number?") == 0) return TOKEN_NUMBER_P;
            break;
            
        case 'o':
            if (strcmp(lexeme, "or") == 0) return TOKEN_OR_OP;
            break;
            
        case 'p':
            if (strcmp(lexeme, "pair?") == 0) return TOKEN_PAIR_P;
            if (strcmp(lexeme, "procedure?") == 0) return TOKEN_PROCEDURE_P;
            break;
            
        case 'q':
            if (strcmp(lexeme, "quasiquote") == 0) return TOKEN_QUASIQUOTE;
            if (strcmp(lexeme, "quote") == 0) return TOKEN_QUOTE;
            break;
            
        case 'r':
            if (strcmp(lexeme, "reverse") == 0) return TOKEN_REVERSE;
            break;
            
        case 's':
            if (strcmp(lexeme, "set!") == 0) return TOKEN_SET;
            if (strcmp(lexeme, "sqrt") == 0) return TOKEN_SQRT;
            if (strcmp(lexeme, "string?") == 0) return TOKEN_STRING_P;
            if (strcmp(lexeme, "symbol?") == 0) return TOKEN_SYMBOL;
            break;
            
        case 'u':
            if (strcmp(lexeme, "unless") == 0) return TOKEN_UNLESS;
            if (strcmp(lexeme, "unquote") == 0) return TOKEN_UNQUOTE;
            break;
            
        case 'w':
            if (strcmp(lexeme, "when") == 0) return TOKEN_WHEN;
            break;
    }
    return TOKEN_IDENTIFIER;
}

token* try_keywords(const char* lexeme) {
    if (!lexeme) return NULL;
    token_type type = check_keyword(lexeme);
    return create_token(lexeme, type);
}

// Process numeric tokens (integers and decimals)
static token* process_number(const char* buf, int* pos) {
    int start = *pos;
    token_type type = TOKEN_DEC;
    
    // Skip minus sign if negative number
    if (buf[*pos] == '-') (*pos)++;
    
    // Process digits before decimal point
    while(isdigit(buf[*pos]) && (*pos - start) < MAX_LEXEME_SIZE) {
        (*pos)++;
    }

    if((*pos - start) >= MAX_LEXEME_SIZE) {
        report_error("Number too long");
        return NULL;
    }

    // Process decimal point and following digits
    if(buf[*pos] == '.') {
        (*pos)++;
        type = TOKEN_REAL;
        while(isdigit(buf[*pos]) && (*pos - start) < MAX_LEXEME_SIZE) {
            (*pos)++;
        }
    }

    int length = *pos - start;
    char* number = (char*)malloc(length + 1);
    strncpy(number, &buf[start], length);
    number[length] = '\0';

    token* t = create_token(number, type);
    free(number);
    return t;
}

// Process string literals
static token* process_string_literal(const char* buf, int* pos) {
    (*pos)++;  // Skip opening quote
    int start = *pos;

    while(buf[*pos] != '"' && buf[*pos] != '\0' && 
          (*pos - start) < MAX_LEXEME_SIZE) {
        (*pos)++;
    }

    if(buf[*pos] == '\0' || (*pos - start) >= MAX_LEXEME_SIZE) {
        report_error("Unterminated or too long string literal");
        return NULL;
    }

    int length = *pos - start;
    char* string = (char*)malloc(length + 1);
    strncpy(string, &buf[start], length);
    string[length] = '\0';
    (*pos)++;  // Skip closing quote

    token* t = create_token(string, TOKEN_STR_LITERAL);
    free(string);
    return t;
}

// Process identifiers and keywords
static token* process_identifier(const char* buf, int* pos) {
    int start = *pos;

    while((isalnum(buf[*pos]) || buf[*pos] == '?' || 
           buf[*pos] == '!' || buf[*pos] == '*' ||
           buf[*pos] == '=' || buf[*pos] == '-' ||
           buf[*pos] == '_') && 
          (*pos - start) < MAX_LEXEME_SIZE) {
        (*pos)++;
    }

    if((*pos - start) >= MAX_LEXEME_SIZE) {
        report_error("Identifier too long");
        return NULL;
    }
    
    int length = *pos - start;
    char* string = (char*)malloc(length + 1);
    strncpy(string, &buf[start], length);
    string[length] = '\0';

    symbol_entry* entry = lookup_symbol(string);
    if (entry) {
        free(string);
        return create_token(entry->lexeme, entry->type);
    } else {
        token* t = try_keywords(string);
        insert_symbol(string, t->type);
        return t;
    }
}

// Process quoted symbols
static token* process_quoted_symbol(const char* buf, int* pos) {
    // Skip the quote mark
    (*pos)++;
    
    int start = *pos;
    while((isalnum(buf[*pos]) || buf[*pos] == '?' || 
           buf[*pos] == '!' || buf[*pos] == '*' ||
           buf[*pos] == '=' || buf[*pos] == '-' ||
           buf[*pos] == '_') && 
          (*pos - start) < MAX_LEXEME_SIZE) {
        (*pos)++;
    }

    if((*pos - start) >= MAX_LEXEME_SIZE) {
        report_error("Symbol too long");
        return NULL;
    }

    int length = *pos - start;
    char* symbol = (char*)malloc(length + 2); // +2 for quote and null terminator
    symbol[0] = '\'';
    strncpy(symbol + 1, &buf[start], length);
    symbol[length + 1] = '\0';

    token* t = create_token(symbol, TOKEN_SYMBOL);
    free(symbol);
    return t;
}

token* next_token(FILE* file) {
    if (!file) return NULL;

    int c;

    // Skip whitespace
    while ((c = get_next_char(file)) != EOF) {
        if (!isspace(c)) {
            unget_char();
            break;
        }
    }

    if (c == EOF) return NULL;

    c = get_next_char(file);
    if (c == EOF) return NULL;

    // Handle single-character operators and special characters
    switch(c) {
        case '(': return create_token("(", TOKEN_LPAREN);
        case ')': return create_token(")", TOKEN_RPAREN);
        case '+': return create_token("+", TOKEN_PLUS);
        case '-': 
            // Check if it's a negative number
            if (isdigit(get_next_char(file))) {
                unget_char();
                return process_number(buffers[current_buffer], &buffer_pointer);
            }
            return create_token("-", TOKEN_MINUS);
        case '*': return create_token("*", TOKEN_MULTIPLY);
        case '/': return create_token("/", TOKEN_DIVIDE);
        case '\'': return process_quoted_symbol(buffers[current_buffer], &buffer_pointer);
        case '`': return create_token("`", TOKEN_BACKQUOTE);
        case ',': return create_token(",", TOKEN_COMMA);
        case '.': return create_token(".", TOKEN_DOT);
        case '<':
            c = get_next_char(file);
            if (c == '=') {
                return create_token("<=", TOKEN_LTE);
            }
            unget_char();
            return create_token("<", TOKEN_LT);
        case '>':
            c = get_next_char(file);
            if (c == '=') {
                return create_token(">=", TOKEN_GTE);
            }
            unget_char();
            return create_token(">", TOKEN_GT);
        case '=': return create_token("=", TOKEN_EQ);
    }

    // Handle numbers
    if (isdigit(c)) {
        unget_char();
        return process_number(buffers[current_buffer], &buffer_pointer);
    }

    // Handle string literals
    if(c == '"') {
        return process_string_literal(buffers[current_buffer], &buffer_pointer);
    }

    // Handle identifiers and keywords
    if(isalpha(c) || c == '?' || c == '!' || c == '*') {
        unget_char();
        return process_identifier(buffers[current_buffer], &buffer_pointer);
    }

    return NULL;
}

const char* token_type_to_string(token_type type) {
    switch(type) {
        // Special forms and syntax keywords
        case TOKEN_AND: return "AND";
        case TOKEN_BEGIN: return "BEGIN";
        case TOKEN_CASE: return "CASE";
        case TOKEN_COND: return "COND";
        case TOKEN_DEFINE: return "DEFINE";
        case TOKEN_DELAY: return "DELAY";
        case TOKEN_DO: return "DO";
        case TOKEN_FORCE: return "FORCE";
        case TOKEN_IF: return "IF";
        case TOKEN_LAMBDA: return "LAMBDA";
        case TOKEN_LET: return "LET";
        case TOKEN_LET_STAR: return "LET*";
        case TOKEN_LETREC: return "LETREC";
        case TOKEN_LETREC_STAR: return "LETREC*";
        case TOKEN_OR: return "OR";
        case TOKEN_QUASIQUOTE: return "QUASIQUOTE";
        case TOKEN_QUOTE: return "QUOTE";
        case TOKEN_SET: return "SET!";
        case TOKEN_UNQUOTE: return "UNQUOTE";
        case TOKEN_UNLESS: return "UNLESS";
        case TOKEN_WHEN: return "WHEN";
        
        // Common procedures
        case TOKEN_APPEND: return "APPEND";
        case TOKEN_APPLY: return "APPLY";
        case TOKEN_CAR: return "CAR";
        case TOKEN_CDR: return "CDR";
        case TOKEN_CONS: return "CONS";
        case TOKEN_DISPLAY: return "DISPLAY";
        case TOKEN_EQ_P: return "EQ?";
        case TOKEN_EQUAL_P: return "EQUAL?";
        case TOKEN_LENGTH: return "LENGTH";
        case TOKEN_LIST: return "LIST";
        case TOKEN_MAP: return "MAP";
        case TOKEN_NOT: return "NOT";
        case TOKEN_NULL_P: return "NULL?";
        case TOKEN_NUMBER_P: return "NUMBER?";
        case TOKEN_PAIR_P: return "PAIR?";
        case TOKEN_PROCEDURE_P: return "PROCEDURE?";
        case TOKEN_REVERSE: return "REVERSE";
        case TOKEN_STRING_P: return "STRING?";
        case TOKEN_SYMBOL: return "SYMBOL";
        
        // Arithmetic operators
        case TOKEN_PLUS: return "PLUS";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_MULTIPLY: return "MULTIPLY";
        case TOKEN_DIVIDE: return "DIVIDE";
        case TOKEN_MOD: return "MODULO";
        case TOKEN_ABS: return "ABS";
        case TOKEN_MAX: return "MAX";
        case TOKEN_MIN: return "MIN";
        case TOKEN_SQRT: return "SQRT";
        case TOKEN_EXPT: return "EXPT";
        
        // Comparison operators
        case TOKEN_EQ: return "EQUAL";
        case TOKEN_LT: return "LESS_THAN";
        case TOKEN_GT: return "GREATER_THAN";
        case TOKEN_LTE: return "LESS_EQUAL";
        case TOKEN_GTE: return "GREATER_EQUAL";
        case TOKEN_NEQ: return "NOT_EQUAL";
        
        // Logical operators
        case TOKEN_NOT_OP: return "NOT";
        case TOKEN_AND_OP: return "AND";
        case TOKEN_OR_OP: return "OR";
        
        // Other token types
        case TOKEN_DEC: return "DECIMAL";
        case TOKEN_REAL: return "REAL";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_STR_LITERAL: return "STRING";
        case TOKEN_DOT: return "DOT";
        case TOKEN_QUOTE_MARK: return "QUOTE";
        case TOKEN_BACKQUOTE: return "BACKQUOTE";
        case TOKEN_COMMA: return "COMMA";
        
        default: return "UNKNOWN";
    }
}

int main() {
    FILE* file = fopen("test.scm", "r");
    if(!file) {
        printf("Failed to open file\n");
        return 1;
    }

    if (fill_buffer(file, 0) <= 0) {
        fclose(file);
        return 1;
    }

    token* t;
    while ((t = next_token(file)) != NULL) {
        if (t->type == TOKEN_DEC) {
            printf("Token: %d\tType: %s\n", t->int_value, token_type_to_string(t->type));
        } else if (t->type == TOKEN_REAL) {
            printf("Token: %f\tType: %s\n", t->real_value, token_type_to_string(t->type));
        } else {
            printf("Token: %s\tType: %s\n", t->lexeme, token_type_to_string(t->type));
        }

        if (t->type != TOKEN_DEC && t->type != TOKEN_REAL) {
            free(t->lexeme);
        }
        free(t);
    }

    cleanup_symbol_table();
    fclose(file);
    return 0;
}
