#ifndef ANALYZER_H
#define ANALYZER_H

#include <stdbool.h>
#include "../parser/parser.h"
#include "symbol_table.h"

#define MAX_BUILTINS 50

typedef enum {
    TYPE_ANY,       // Accepts any type
    TYPE_NUMBER,    // Integer or real number
    TYPE_STRING,    // String literal
    TYPE_BOOLEAN,   // True or false
    TYPE_PAIR,      // Cons cell / list
    TYPE_PROCEDURE  // Function/lambda
} value_type;

typedef struct {
    const char* name;
    int min_arity;      // Minimum number of arguments (-1 for special handling)
    int max_arity;      // Maximum number of arguments (-1 for unlimited)
    value_type arg_type; // Expected type for arguments (TYPE_ANY for mixed)
} builtin_info;

typedef struct analyzer {
    scope* current_scope;
    token* builtin_tokens[50];
    int builtin_token_count;
} analyzer;

analyzer* init_analyzer();

void free_analyzer(analyzer* a);

bool analyze_ast(analyzer* a,ast_node* root);

#endif // ANALYZER_H