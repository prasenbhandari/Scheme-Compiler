#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "../scanner/token.h"
#include <stdbool.h>

typedef struct {
    FILE* file;
    token* current;
    token* next;
    bool panic_mode;
} parser;

typedef enum {
    NODE_ATOM,
    NODE_LIST,
    NODE_NIL
} node_type;

typedef struct ast_node {
    node_type type;
    token* token;
    struct ast_node* car;
    struct ast_node* cdr;
} ast_node;


parser* init_parser(FILE* file);
void free_parser(parser* p);
token* peek(parser* p);
token* advance(parser* p);
bool match(parser* p, token_type type);
token* expect(parser* p, token_type type);

ast_node* parse_expression(parser* p);
ast_node* parse_list(parser* p);
ast_node* parse_atom(parser* p);

#endif