#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "token.h"
#include "utils/buffer.h"  // For get_current_line() and get_current_column()


Token* create_token(const char* lexeme, TokenType type) {
    Token* temp = malloc(sizeof(Token));
    if (!temp) {
        fprintf(stderr, "Memory allocation failed for token\n");
        exit(1);
    }

    temp->type = type;
    temp->line = get_current_line();
    temp->column = get_current_column();
    
    if(type == TOKEN_DEC) {
        temp->int_value = strtoll(lexeme, NULL, 10);
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


void free_token(Token* t) {
    if(t->type != TOKEN_DEC && t->type != TOKEN_REAL) {
        free(t->lexeme);
    }
    free(t);
}


TokenType check_keyword(const char* lexeme) {
    switch(lexeme[0]) {

        case 'a':
            if (strcmp(lexeme, "and") == 0) return TOKEN_AND;
            break;

        case 'b':
            if (strcmp(lexeme, "begin") == 0) return TOKEN_BEGIN;
            break;

        case 'c':
            if (strcmp(lexeme, "case") == 0) return TOKEN_CASE;
            if (strcmp(lexeme, "cond") == 0) return TOKEN_COND;
            break;

        case 'd':
            if (strcmp(lexeme, "define") == 0) return TOKEN_DEFINE;
            if (strcmp(lexeme, "delay") == 0) return TOKEN_DELAY;
            if (strcmp(lexeme, "do") == 0) return TOKEN_DO;
            break;

        case 'e':
            if (strcmp(lexeme, "else") == 0) return TOKEN_ELSE;
            break;

        case 'i':
            if (strcmp(lexeme, "if") == 0) return TOKEN_IF;
            break;

        case 'l':
            if (strcmp(lexeme, "lambda") == 0) return TOKEN_LAMBDA;
            if (strcmp(lexeme, "let") == 0) return TOKEN_LET;
            if (strcmp(lexeme, "let*") == 0) return TOKEN_LET_STAR;
            if (strcmp(lexeme, "letrec") == 0) return TOKEN_LETREC;
            if (strcmp(lexeme, "letrec*") == 0) return TOKEN_LETREC_STAR;
            break;

        case 'o':
            if (strcmp(lexeme, "or") == 0) return TOKEN_OR;
            break;

        case 'q':
            if (strcmp(lexeme, "quasiquote") == 0) return TOKEN_QUASIQUOTE;
            if (strcmp(lexeme, "quote") == 0) return TOKEN_QUOTE;
            break;

        case 's':
            if (strcmp(lexeme, "set!") == 0) return TOKEN_SET;
            break;

        case 'u':
            if (strcmp(lexeme, "unless") == 0) return TOKEN_UNLESS;
            if (strcmp(lexeme, "unquote") == 0) return TOKEN_UNQUOTE;
            break;

        case 'w':
            if (strcmp(lexeme, "when") == 0) return TOKEN_WHEN;
            break;
            
        case '\'':
            return TOKEN_SYMBOL;
            break;
    }
    return TOKEN_IDENTIFIER;
}


const char* token_type_to_string(TokenType type) {
    switch(type) {
        // Special forms (keywords)
        case TOKEN_AND: return "AND";
        case TOKEN_BEGIN: return "BEGIN";
        case TOKEN_CASE: return "CASE";
        case TOKEN_COND: return "COND";
        case TOKEN_DEFINE: return "DEFINE";
        case TOKEN_DELAY: return "DELAY";
        case TOKEN_DO: return "DO";
        case TOKEN_ELSE: return "ELSE";
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
        
        // Other token types
        case TOKEN_DEC: return "DECIMAL";
        case TOKEN_REAL: return "REAL";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_STR_LITERAL: return "STRING";
        case TOKEN_DOT: return "DOT";
        case TOKEN_QUOTE_MARK: return "QUOTE_MARK";
        case TOKEN_BACKQUOTE: return "BACKQUOTE";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_SYMBOL: return "SYMBOL";
        
        default: return "UNKNOWN";
    }
}
