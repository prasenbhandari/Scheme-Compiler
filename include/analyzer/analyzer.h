#ifndef ANALYZER_H
#define ANALYZER_H

#include <stdbool.h>
#include "../parser/parser.h"
#include "symbol_table.h"

#define MAX_BUILTINS 50

typedef struct analyzer {
    scope* current_scope;
    token* builtin_tokens[50];
    int builtin_token_count;
} analyzer;

analyzer* init_analyzer();

void free_analyzer(analyzer* a);

bool analyze_ast(analyzer* a,ast_node* root);

#endif // ANALYZER_H