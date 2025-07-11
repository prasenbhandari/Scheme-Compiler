#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scanner/scanner.h"
#include "scanner/token.h"
#include "utils/buffer.h"
#include "utils/error.h"

#define MAX_LEXEME_SIZE 256

// Forward declarations for internal functions
static token* process_number(const char* buf, int* pos);
static token* process_string_literal(const char* buf, int* pos);
static token* process_identifier(const char* buf, int* pos);
static token* process_quoted_symbol(const char* buf, int* pos);
static token* try_keywords(const char* lexeme);

// Processing functions implementations
static token* process_number(const char* buf, int* pos) {
    int start = *pos;
    token_type type = TOKEN_DEC;
    
    if (buf[*pos] == '-') (*pos)++;
    
    while(isdigit(buf[*pos]) && (*pos - start) < MAX_LEXEME_SIZE) {
        (*pos)++;
    }

    if(buf[*pos] == '.') {
        (*pos)++;
        type = TOKEN_REAL;
        while(isdigit(buf[*pos]) && (*pos - start) < MAX_LEXEME_SIZE) {
            (*pos)++;
        }
    }

    int length = *pos - start;
    if(length >= MAX_LEXEME_SIZE) {
        report_error("Number too long");
        return NULL;
    }

    char* number = malloc(length + 1);
    strncpy(number, &buf[start], length);
    number[length] = '\0';

    token* t = create_token(number, type);
    free(number);
    return t;
}

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
    char* string = malloc(length + 1);
    strncpy(string, &buf[start], length);
    string[length] = '\0';
    (*pos)++;  // Skip closing quote

    token* t = create_token(string, TOKEN_STR_LITERAL);
    free(string);
    return t;
}

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
    char* string = malloc(length + 1);
    strncpy(string, &buf[start], length);
    string[length] = '\0';

    token* t = try_keywords(string);
    free(string);
    return t;
}

token* peek(FILE* file) {
    // Save complete state
    long file_pos = ftell(file);
    buffer_state buf_state = get_buffer_state();
    int current_line = get_current_line();
    int current_column = get_current_column();
    
    // Get next token
    token* t = next_token(file);
    
    // Restore complete state
    fseek(file, file_pos, SEEK_SET);
    restore_buffer_state(buf_state);
    set_position(current_line, current_column);
    
    return t;
}

static token* process_quoted_symbol(const char* buf, int* pos) {
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
    char* symbol = malloc(length + 2);
    symbol[0] = '\'';
    strncpy(symbol + 1, &buf[start], length);
    symbol[length + 1] = '\0';

    token* t = create_token(symbol, TOKEN_SYMBOL);
    free(symbol);
    return t;
}


static token* try_keywords(const char* lexeme) {
    if (!lexeme) return NULL;
    token_type type = check_keyword(lexeme);
    return create_token(lexeme, type);
}

token* next_token(FILE* file) {
    if (!file) return NULL;

    int c;
    char* current_buf = get_current_buffer();
    int* pos = get_buffer_pos();

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
            if (isdigit(get_next_char(file))) {
                unget_char();
                return process_number(get_current_buffer(), pos);
            }
            return create_token("-", TOKEN_MINUS);
        case '*': return create_token("*", TOKEN_MULTIPLY);
        case '/': return create_token("/", TOKEN_DIVIDE);
        case '\'': return process_quoted_symbol(get_current_buffer(), pos);
        case '`': return create_token("`", TOKEN_BACKQUOTE);
        case ',': return create_token(",", TOKEN_COMMA);
        case '.': return create_token(".", TOKEN_DOT);
        case '<':
            c = get_next_char(file);
            if (c == '=') return create_token("<=", TOKEN_LTE);
            unget_char();
            return create_token("<", TOKEN_LT);
        case '>':
            c = get_next_char(file);
            if (c == '=') return create_token(">=", TOKEN_GTE);
            unget_char();
            return create_token(">", TOKEN_GT);
        case '=': return create_token("=", TOKEN_EQ);
    }

    // Handle numbers, strings, and identifiers
    if (isdigit(c)) {
        unget_char();
        return process_number(current_buf, pos);
    }

    if(c == '"') {
        return process_string_literal(current_buf, pos);
    }

    if(isalpha(c) || c == '?' || c == '!' || c == '*') {
        unget_char();
        return process_identifier(current_buf, pos);
    }

    return NULL;
}

void init_scanner(void) {
    init_buffer();
    init_error();
}

void cleanup_scanner(void) {
    cleanup_buffer();
    cleanup_error();
}
