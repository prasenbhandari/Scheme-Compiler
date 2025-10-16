#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "analyzer.h"
#include "symbol_table.h"
#include "error.h"


static void analyze_node(analyzer* a, ast_node* node);


static token* create_builtin_token(const char* name){
    token* t = (token*)malloc(sizeof(token));

    if (!t){
        exit(1);
    }

    t->lexeme = strdup(name);
    t->type = TOKEN_IDENTIFIER;
    return t;
}


analyzer* init_analyzer() {

    analyzer* a = (analyzer*)malloc(sizeof(analyzer));

    a->current_scope = init_scope(NULL);

    a->builtin_token_count = 0;

    const char* builtins[] = {
        // I/O procedures
        "display", "newline", "read", "write", "print",
        
        // Arithmetic operators
        "+", "-", "*", "/", "modulo", "remainder", "quotient",
        "abs", "max", "min", "sqrt", "expt",
        
        // Comparison operators  
        "=", "<", ">", "<=", ">=",
        
        // List manipulation
        "car", "cdr", "cons", "list", "append", "reverse", "length",
        "map", "filter", "reduce", "apply",
        
        // Type predicates
        "null?", "pair?", "list?", "symbol?", "number?", "integer?",
        "string?", "boolean?", "procedure?", "atom?",
        
        // Equality
        "eq?", "eqv?", "equal?",
        
        // Logical (not is a procedure, not a special form!)
        "not",
        
        // Numeric predicates
        "even?", "odd?", "zero?",
        
        NULL
    };

    for(int i = 0; builtins[i] != NULL; i++){
        token* t = create_builtin_token(builtins[i]);
        add_symbol(a->current_scope, t);

        if(a->builtin_token_count < MAX_BUILTINS){
            a->builtin_tokens[a->builtin_token_count++] = t;
        }
    }

    return a;
}


void free_analyzer(analyzer* a) {
    for(int i = 0; i < a->builtin_token_count;i++){
        free(a->builtin_tokens[i]->lexeme);
        free(a->builtin_tokens[i]);
    }

    free_scope(a->current_scope);

    free(a);
}


bool analyze_ast(analyzer* a, ast_node* root){
    analyze_node(a, root);
    return !had_error();
}


static void analyze_node(analyzer* a, ast_node* node){
    if (node == NULL || node->type == NODE_NIL) return;

    switch(node->type){
        case NODE_ATOM:{
            if(node->token->type == TOKEN_IDENTIFIER){
                symbol* sym = find_symbol(a->current_scope, node->token);
                if (!sym){
                    report_error(-1, -1, "Undefined identifier: %s", node->token->lexeme);
                }
            }
            break;
        }
        case NODE_LIST: {
            analyze_node(a, node->car);
            
            ast_node* arg = node->cdr;
            while(arg && arg->type != NODE_NIL){
                analyze_node(a, arg->car);
                arg = arg->cdr;
            }
            break;
        }
    }
}