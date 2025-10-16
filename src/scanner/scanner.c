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
static token* process_number(void);
static token* process_string_literal(void);
static token* process_identifier(void);
static token* process_quoted_symbol(void);
static token* try_keywords(const char* lexeme);

// Processing functions implementations
static token* process_number(void) {
    int start = *get_buffer_pos();
    token_type type = TOKEN_DEC;
    
    if (peek_char() == '-') get_next_char();
    
    while(isdigit(peek_char())) {
        get_next_char();
    }

    if(peek_char() == '.') {
        get_next_char();
        type = TOKEN_REAL;
        while(isdigit(peek_char())) {
            get_next_char();
        }
    }

    int length = *get_buffer_pos() - start;
    if(length >= MAX_LEXEME_SIZE) {
        report_error(get_current_line(), get_current_column(), "Number too long");
        return NULL;
    }

    char* number = strndup(get_current_buffer() + start, length);
    token* t = create_token(number, type);
    free(number);
    return t;
}

static token* process_string_literal(void) {
    int start_line = get_current_line();
    int start_column = get_current_column();
    get_next_char(); // Skip opening quote
    int start = *get_buffer_pos();

    while(peek_char() != '"' && peek_char() != '\n' && peek_char() != EOF) {
        get_next_char();
    }

    if(peek_char() == '\n' || peek_char() == EOF) {
        report_error(start_line, start_column, "Unterminated string literal");
        return NULL;
    }

    int length = *get_buffer_pos() - start;
    char* string = strndup(get_current_buffer() + start, length);
    get_next_char(); // Skip closing quote

    token* t = create_token(string, TOKEN_STR_LITERAL);
    free(string);
    return t;
}

static token* process_identifier(void) {
    int start = *get_buffer_pos();

    while(isalnum(peek_char()) || strchr("?!*=-_", peek_char())) {
        get_next_char();
    }
    
    int length = *get_buffer_pos() - start;
    char* string = strndup(get_current_buffer() + start, length);

    token* t = try_keywords(string);
    free(string);
    return t;
}

static token* process_quoted_symbol(void) {
    get_next_char(); // Skip quote
    int start = *get_buffer_pos();
    
    while(isalnum(peek_char()) || strchr("?!*=-_", peek_char())) {
        get_next_char();
    }

    int length = *get_buffer_pos() - start;
    char* symbol = malloc(length + 2);
    symbol[0] = '\'';
    strncpy(symbol + 1, get_current_buffer() + start, length);
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

token* next_token(void) {
    char c = skip_whitespace();
    if (c == EOF) return NULL;

    // Consume the character we peeked at
    get_next_char();

    if (c == '(') return create_token("(", TOKEN_LPAREN);
    if (c == ')') return create_token(")", TOKEN_RPAREN);
    if (c == '`') return create_token("`", TOKEN_BACKQUOTE);
    if (c == ',') return create_token(",", TOKEN_COMMA);
    if (c == '.') return create_token(".", TOKEN_DOT);

    if (c == '+') return create_token("+", TOKEN_IDENTIFIER);
    if (c == '*') return create_token("*", TOKEN_IDENTIFIER);
    if (c == '/') return create_token("/", TOKEN_IDENTIFIER);
    if (c == '=') return create_token("=", TOKEN_IDENTIFIER);

    if (c == '-') {
        if (isdigit(peek_char())) {
            return process_number();
        }
        return create_token("-", TOKEN_IDENTIFIER);
    }

    if (c == '<') {
        if (peek_char() == '=') {
            get_next_char();
            return create_token("<=", TOKEN_IDENTIFIER);
        }
        return create_token("<", TOKEN_IDENTIFIER);
    }

    if (c == '>') {
        if (peek_char() == '=') {
            get_next_char();
            return create_token(">=", TOKEN_IDENTIFIER);
        }
        return create_token(">", TOKEN_IDENTIFIER);
    }

    if (c == '\'') {
        return process_quoted_symbol();
    }

    if (isdigit(c)) {
        unget_char(); // Put the digit back for process_number to read
        return process_number();
    }

    if (c == '"') {
        unget_char(); // Put the quote back for process_string_literal to read
        return process_string_literal();
    }

    if (isalpha(c) || strchr("?!*=-_", c)) {
        unget_char(); // Put the character back for process_identifier to read
        return process_identifier();
    }

    report_error(get_current_line(), get_current_column(), "Unexpected character '%c'", c);
    return NULL;
}

void init_scanner(FILE *file) {
    init_buffer(file);
}

void cleanup_scanner(void) {
    cleanup_buffer();
    cleanup_error();
}
