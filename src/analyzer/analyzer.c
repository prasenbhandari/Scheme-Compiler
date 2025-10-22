#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "analyzer.h"
#include "symbol_table.h"
#include "error.h"


static const builtin_info BUILTIN_TABLE[] = {
    // I/O procedures
    {"display", 1, 1, TYPE_ANY},

    // Arithmetic operators
    {"+", 0, -1, TYPE_NUMBER},  // (+) => 0, (+ 1) => 1, (+ 1 2 3) => 6
    {"-", 1, -1, TYPE_NUMBER},  // (- 5) => -5, (- 10 3) => 7
    {"*", 0, -1, TYPE_NUMBER},  // (*) => 1
    {"/", 1, -1, TYPE_NUMBER},  // (/ 10) => 1/10, (/ 10 2) => 5

    // Terminator
    {NULL, 0, 0, TYPE_ANY}
};


static const builtin_info* get_builtin_info(const char* name){
    for(int i = 0; BUILTIN_TABLE[i].name != NULL; i++){
        if (strcmp(BUILTIN_TABLE[i].name, name) == 0){
            return &BUILTIN_TABLE[i];
        }
    }
    return NULL;
}


static int count_args(ast_node* arg_list){
    int count = 0;
    ast_node* current = arg_list;

    while(current && current->type != NODE_NIL){
        count++;
        current = current->cdr;
    }
    return count;
}


static value_type get_node_type(ast_node* node){
    if (node == NULL) return TYPE_ANY;

    switch(node->type){
        case NODE_ATOM:
            switch(node->token->type){
                case TOKEN_DEC:
                case TOKEN_REAL:
                    return TYPE_NUMBER;
                case TOKEN_STR_LITERAL:
                    return TYPE_STRING;
                case TOKEN_IDENTIFIER:
                    return TYPE_ANY;
                default:
                    return TYPE_ANY;
            }
        case NODE_LIST:
            return TYPE_PAIR;
        case NODE_NIL:
            return TYPE_PAIR;
        default:
            return TYPE_ANY;
    }
}


static const char* type_to_string(value_type type){
    switch(type){
        case TYPE_ANY: return "any";
        case TYPE_NUMBER: return "number";
        case TYPE_STRING: return "string";
        case TYPE_BOOLEAN: return "boolean";
        case TYPE_PAIR: return "pair";
        case TYPE_PROCEDURE: return "procedure";
        default: return "unknown";
    }
}


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
            ast_node* operator = node->car;
            ast_node* arg = node->cdr;

            analyze_node(a, operator);

            if (operator && operator->type == NODE_ATOM && operator->token->type == TOKEN_IDENTIFIER) {
                const builtin_info* info = get_builtin_info(operator->token->lexeme);

                if(info) {
                    int arg_count = count_args(arg);

                    if(info->min_arity != -1 && arg_count < info->min_arity){
                        report_error(-1, -1, "Too few arguments to '%s'. Expected at least %d, got %d.",
                                     info->name, info->min_arity, arg_count);
                    }

                    if(info->max_arity != -1 && arg_count > info->max_arity){
                        report_error(-1, -1, "Too many arguments to '%s'. Expected at most %d, got %d.",
                                     info->name, info->max_arity, arg_count);
                    }

                    if (info->arg_type != TYPE_ANY){
                        ast_node* temp_arg = arg;
                        int position = 1;

                        while(temp_arg && temp_arg->type != NODE_NIL){
                            value_type arg_type = get_node_type(temp_arg->car);
                            if(arg_type != TYPE_ANY && arg_type != info->arg_type){
                                report_error(-1, -1, "Argument %d to '%s' has incorrect type. Expected %s, got %s.",
                                             position, info->name, type_to_string(info->arg_type), type_to_string(arg_type));
                            }
                            temp_arg = temp_arg->cdr;
                            position++;
                        }
                    }
                }
            }

            while(arg && arg->type != NODE_NIL){
                analyze_node(a, arg->car);
                arg = arg->cdr;
            }
            break;
        }
    }
}