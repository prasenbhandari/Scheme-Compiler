#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "../scanner/token.h"

typedef struct ast_node {
    token* token;
    struct ast_node* left;
    struct ast_node* right;
} ast_node;



#endif