#include <stdio.h>
#include <stdlib.h>
#include "scanner.h"
#include "token.h"
#include "error.h"
#include "parser.h"


parser* init_parser(FILE* file) {
    parser* p = (parser*)malloc(sizeof(parser));
    p->file = file;

    init_scanner(file);

    p->current = next_token();
    p->next = next_token();

    return p;
}

// Helper function to create a NIL node
ast_node* create_nil_node() {
    ast_node* nil = (ast_node*)malloc(sizeof(ast_node));
    nil->type = NODE_NIL;
    nil->token = NULL;
    nil->car = NULL;
    nil->cdr = NULL;
    return nil;
}

token* peek(parser* p) {
    if (!p) return NULL;

    return p->current;
}

token* advance(parser* p) {
    if (!p) return NULL;

    token* temp = p->current;
    p->current = p->next;
    p->next = next_token();

    return temp;
}

bool match(parser *p, token_type type){
    if (!p || !p->current) return false;

    if(p->current->type == type){
        return true;
    }

    return false;
}

token* expect(parser* p, token_type type) {
    if (!match(p, type)) {
        // Error: expected type but got something else
        fprintf(stderr, "Expected token type %s but got %s\n",
                token_type_to_string(type),
                p->current ? token_type_to_string(p->current->type) : "EOF");
        return NULL;
    }
    return advance(p);
}

ast_node* parse_atom(parser* p){
    ast_node* node = (ast_node*)malloc(sizeof(ast_node));
    node->type = NODE_ATOM;
    node->token = p->current;
    node->car = NULL;
    node->cdr = NULL;
    advance(p);
    
    return node;
}


ast_node* parse_expression(parser* p){
    if(match(p, TOKEN_LPAREN)){
        return parse_list(p);
    }else{
        return parse_atom(p);
    }
}

ast_node* parse_list(parser* p){
    expect(p, TOKEN_LPAREN);
    
    ast_node* node = (ast_node*)malloc(sizeof(ast_node));
    node->type = NODE_LIST;
    node->car = parse_expression(p);
    node->cdr = create_nil_node();  // Initialize with NIL

    ast_node* current_list_node = node;

    while(!match(p, TOKEN_RPAREN)){
        // Parse the next argument
        ast_node* arg = parse_expression(p);
        
        // Create a new list node to hold this argument
        ast_node* new_list_node = (ast_node*)malloc(sizeof(ast_node));
        new_list_node->type = NODE_LIST;
        new_list_node->car = arg;
        new_list_node->cdr = create_nil_node();  // End with NIL, not NULL
        
        // Link it to the previous node's cdr
        current_list_node->cdr = new_list_node;
        
        // Move forward in the list
        current_list_node = new_list_node;
    }

    expect(p, TOKEN_RPAREN);
    return node;
}