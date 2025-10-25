#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "../scanner/token.h"
#include <stdbool.h>

typedef struct {
    FILE* file;
    Token* current;
    Token* next;
    bool panic_mode;
} Parser;

typedef enum {
    NODE_ATOM,
    NODE_LIST,
    NODE_NIL
} NodeType;

typedef struct ast_node {
    NodeType type;
    Token* token;
    struct ast_node* car;
    struct ast_node* cdr;
} AstNode;


Parser* init_parser(FILE* file);
void free_parser(Parser* p);
Token* peek(Parser* p);
Token* advance(Parser* p);
bool match(Parser* p, TokenType type);
Token* expect(Parser* p, TokenType type);

AstNode* parse_expression(Parser* p);
AstNode* parse_list(Parser* p);
AstNode* parse_atom(Parser* p);
void free_ast(AstNode* node);

#endif