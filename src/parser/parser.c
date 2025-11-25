#include <stdio.h>
#include <stdlib.h>
#include "../../include/scanner/scanner.h"
#include "../../include/scanner/token.h"
#include "../../include/utils/error.h"
#include "../../include/utils/buffer.h"
#include "../../include/parser/parser.h"


Parser* init_parser(FILE* file) {
    Parser* p = (Parser*)malloc(sizeof(Parser));
    p->file = file;
    p->panic_mode = false;

    init_scanner(file);

    p->current = next_token();
    p->next = next_token();

    return p;
}


void free_parser(Parser* p) {
    if (!p) return;
    
    // Free current and next tokens if they exist
    if (p->current) {
        free_token(p->current);
    }
    if (p->next) {
        free_token(p->next);
    }
    
    // Free the parser structure itself
    free(p);
}


static void synchronize(Parser* p){
    if (!p) return;

    while(p->current != NULL){
        if (match(p, TOKEN_RPAREN)){
            p->panic_mode = false;
            return;
        }

        if (match(p, TOKEN_LPAREN)){
            p->panic_mode = false;
            return;
        }

        advance(p);
    }
}


AstNode* create_nil_node() {
    AstNode* nil = (AstNode*)malloc(sizeof(AstNode));
    nil->type = NODE_NIL;
    nil->token = NULL;
    nil->car = NULL;
    nil->cdr = NULL;
    nil->line = -1;    // NIL has no source location
    nil->column = -1;
    return nil;
}


Token* peek(Parser* p) {
    if (!p) return NULL;

    return p->current;
}


Token* advance(Parser* p) {
    if (!p) return NULL;

    Token* temp = p->current;
    p->current = p->next;
    p->next = next_token();

    return temp;
}

bool match(Parser *p, TokenType type){
    if (!p || !p->current) return false;

    if(p->current->type == type){
        return true;
    }

    return false;
}


Token* expect(Parser* p, TokenType type) {
    if (!match(p, type)) {
        if (!p->panic_mode) {
            if (!p->current) {
                report_error(get_current_line(), get_current_column(),
                            "Unexpected end of file. Expected '%s'",
                            token_type_to_string(type));
            } else {
                report_error(get_current_line(), get_current_column(),
                            "Expected '%s' but got '%s'",
                            token_type_to_string(type),
                            token_type_to_string(p->current->type));
            }
            p->panic_mode = true;
        }
        return NULL;
    }
    return advance(p);
}


AstNode* parse_atom(Parser* p){
    AstNode* node = (AstNode*)malloc(sizeof(AstNode));
    node->type = NODE_ATOM;
    node->token = p->current;
    node->car = NULL;
    node->cdr = NULL;
    node->line = p->current->line;      // Copy position from token
    node->column = p->current->column;
    advance(p);
    
    return node;
}


AstNode* parse_expression(Parser* p){
    if (!p->current) {
        report_error(get_current_line(), get_current_column(),
                    "Unexpected end of file while parsing expression");
        return NULL;
    }
    
    if (match(p, TOKEN_RPAREN)) {
        if (!p->panic_mode) {
            report_error(get_current_line(), get_current_column(),
                    "Unexpected ')'. Expected an expression");
            p->panic_mode = true;
        }
        advance(p);
        synchronize(p);
        return NULL;
    }
    
    if (match(p, TOKEN_QUOTE_MARK)) {
        Token* quote_token = advance(p);  // Consume the quote mark
        AstNode* quoted_expr = parse_expression(p);  // Parse the expression to quote
        
        if (!quoted_expr) {
            free_token(quote_token);
            return NULL;
        }
        
        // Build (quote expr) as a list
        AstNode* quote_list = (AstNode*)malloc(sizeof(AstNode));
        quote_list->type = NODE_LIST;
        quote_list->line = quote_token->line;
        quote_list->column = quote_token->column;
        
        // Create the 'quote' atom
        AstNode* quote_atom = (AstNode*)malloc(sizeof(AstNode));
        quote_atom->type = NODE_ATOM;
        quote_atom->token = create_token("quote", TOKEN_QUOTE);
        quote_atom->car = NULL;
        quote_atom->cdr = NULL;
        quote_atom->line = quote_token->line;
        quote_atom->column = quote_token->column;
        
        // Build cons structure: (quote . (expr . ()))
        quote_list->car = quote_atom;
        
        AstNode* expr_list = (AstNode*)malloc(sizeof(AstNode));
        expr_list->type = NODE_LIST;
        expr_list->car = quoted_expr;
        expr_list->cdr = create_nil_node();
        expr_list->line = quoted_expr->line;
        expr_list->column = quoted_expr->column;
        
        quote_list->cdr = expr_list;
        
        free_token(quote_token);
        return quote_list;
    }
    
    if(match(p, TOKEN_LPAREN)){
        return parse_list(p);
    }else{
        return parse_atom(p);
    }
}


AstNode* parse_list(Parser* p){
    Token* start_token = p->current;  // Save opening paren position
    
    if (!expect(p, TOKEN_LPAREN)) {
        return NULL;
    }
    
    // Check for empty list ()
    if (match(p, TOKEN_RPAREN)) {
        advance(p);
        return create_nil_node();
    }
    
    AstNode* node = (AstNode*)malloc(sizeof(AstNode));
    node->type = NODE_LIST;
    node->token = NULL;
    node->line = start_token->line;      // Copy position from opening paren
    node->column = start_token->column;
    node->car = parse_expression(p);
    
    if (!node->car) {
        free(node);
        return NULL;
    }
    
    node->cdr = create_nil_node();

    AstNode* current_list_node = node;

    while(!match(p, TOKEN_RPAREN)){
        // Check for unexpected EOF
        if (!p->current) {
            report_error(get_current_line(), get_current_column(),
                        "Unexpected end of file. Expected ')' to close list");
            return NULL;
        }

        if (match(p, TOKEN_DOT)) {
            advance(p);

            AstNode* tail = parse_expression(p);
            if (!tail) return NULL;

            free_ast(current_list_node->cdr);
            current_list_node->cdr = tail;

            if (!expect(p, TOKEN_RPAREN)) {
                return NULL;
            }
            return node;
        }
        
        // Parse the next argument
        AstNode* arg = parse_expression(p);
        
        // Check if parse_expression failed
        if (!arg) {
            return NULL;
        }
        
        // Create a new list node to hold this argument
        AstNode* new_list_node = (AstNode*)malloc(sizeof(AstNode));
        new_list_node->type = NODE_LIST;
        new_list_node->car = arg;
        new_list_node->cdr = create_nil_node();
        new_list_node->line = arg->line;      // Inherit position from argument
        new_list_node->column = arg->column;
        
        // Link it to the previous node's cdr
        current_list_node->cdr = new_list_node;
        
        // Move forward in the list
        current_list_node = new_list_node;
    }

    if (!expect(p, TOKEN_RPAREN)) {
        return NULL;
    }
    
    return node;
}

void free_ast(AstNode* node){
    if (!node) return;

    switch (node->type) {
        case NODE_NIL:
            free(node);
            return;
        case NODE_ATOM:
            if (node->token) free_token(node->token);
            free(node);
            return;
        case NODE_LIST:
            free_ast(node->car);
            free_ast(node->cdr);
            free(node);
            return;
    }
    
}


AstNode* get_arg(AstNode* args, int n) {
    AstNode* cur = args;
    for (int i = 0; i < n; i++) {
        if (!cur || cur->type == NODE_NIL) return NULL;
        cur = cur->cdr;
    }
    return cur ? cur->car : NULL;
}