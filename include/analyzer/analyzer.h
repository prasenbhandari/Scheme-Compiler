#ifndef ANALYZER_H
#define ANALYZER_H

#include <stdbool.h>
#include "../parser/parser.h"
#include "../vm/value.h"  // Use VM's ValueType as single source of truth
#include "symbol_table.h"

#define MAX_BUILTINS 50

typedef struct {
    const char* name;
    int min_arity;      // Minimum number of arguments (-1 for special handling)
    int max_arity;      // Maximum number of arguments (-1 for unlimited)
    ValueType arg_type; // Expected type for arguments (VAL_ANY for mixed)
} BuiltinInfo;

typedef struct Analyzer {
    Scope* current_scope;
    Token* builtin_tokens[MAX_BUILTINS]; 
    int builtin_token_count;
} Analyzer;

Analyzer* init_analyzer();

void free_analyzer(Analyzer* a);

bool analyze_ast(Analyzer* a,AstNode* root);

#endif // ANALYZER_H