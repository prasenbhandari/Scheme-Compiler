#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "analyzer.h"
#include "symbol_table.h"
#include "error.h"


static const BuiltinInfo BUILTIN_TABLE[] = {
    // I/O procedures
    {"display", 1, 1, VAL_ANY},

    // Arithmetic operators
    {"+", 0, -1, VAL_NUMBER},  // (+) => 0, (+ 1) => 1, (+ 1 2 3) => 6
    {"-", 1, -1, VAL_NUMBER},  // (- 5) => -5, (- 10 3) => 7
    {"*", 0, -1, VAL_NUMBER},  // (*) => 1
    {"/", 1, -1, VAL_NUMBER},  // (/ 10) => 1/10, (/ 10 2) => 5

    // Comparison operators
    {"<", 2, 2, VAL_NUMBER},
    {">", 2, 2, VAL_NUMBER},
    {"=", 2, 2, VAL_NUMBER},
    {"<=", 2, 2, VAL_NUMBER},
    {">=", 2, 2, VAL_NUMBER},
    {"!=", 2, 2, VAL_NUMBER},

    // Terminator
    {NULL, 0, 0, VAL_ANY}
};


static const BuiltinInfo* get_builtin_info(const char* name){
    for(int i = 0; BUILTIN_TABLE[i].name != NULL; i++){
        if (strcmp(BUILTIN_TABLE[i].name, name) == 0){
            return &BUILTIN_TABLE[i];
        }
    }
    return NULL;
}


static int count_args(AstNode* arg_list){
    int count = 0;
    AstNode* current = arg_list;

    while(current && current->type != NODE_NIL){
        count++;
        current = current->cdr;
    }
    return count;
}


static ValueType get_node_type(AstNode* node){
    if (node == NULL) return VAL_ANY;

    switch(node->type){
        case NODE_ATOM:
            switch(node->token->type){
                case TOKEN_DEC:
                case TOKEN_REAL:
                    return VAL_NUMBER;
                case TOKEN_STR_LITERAL:
                    return VAL_STRING;
                case TOKEN_IDENTIFIER:
                    return VAL_ANY;
                default:
                    return VAL_ANY;
            }
        case NODE_LIST:
            return VAL_ANY;
        case NODE_NIL:
            return VAL_PAIR;
        default:
            return VAL_ANY;
    }
}


static const char* type_to_string(ValueType type){
    switch(type){
        case VAL_ANY: return "any";
        case VAL_NUMBER: return "number";
        case VAL_STRING: return "string";
        case VAL_BOOL: return "boolean";
        case VAL_PAIR: return "pair";
        case VAL_PROCEDURE: return "procedure";
        case VAL_NIL: return "nil";
        default: return "unknown";
    }
}


static void analyze_node(Analyzer* a, AstNode* node);


static Token* create_builtin_token(const char* name){
    Token* t = (Token*)malloc(sizeof(Token));

    if (!t){
        exit(1);
    }

    t->lexeme = strdup(name);
    t->type = TOKEN_IDENTIFIER;
    return t;
}


Analyzer* init_analyzer() {

    Analyzer* a = (Analyzer*)malloc(sizeof(Analyzer));

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
        Token* t = create_builtin_token(builtins[i]);
        add_symbol(a->current_scope, t);

        if(a->builtin_token_count < MAX_BUILTINS){
            a->builtin_tokens[a->builtin_token_count++] = t;
        }
    }

    return a;
}


void free_analyzer(Analyzer* a) {
    for(int i = 0; i < a->builtin_token_count;i++){
        free(a->builtin_tokens[i]->lexeme);
        free(a->builtin_tokens[i]);
    }

    free_scope(a->current_scope);

    free(a);
}


bool analyze_ast(Analyzer* a, AstNode* root){
    analyze_node(a, root);
    return !had_error();
}


static void analyze_node(Analyzer* a, AstNode* node){
    if (node == NULL || node->type == NODE_NIL) return;

    switch(node->type){
        case NODE_ATOM:{
            if(node->token->type == TOKEN_IDENTIFIER){
                Symbol* sym = find_symbol(a->current_scope, node->token);
                if (!sym){
                    report_error(node->line, node->column, 
                                "Undefined identifier: %s", node->token->lexeme);
                }
            }
            break;
        }
        case NODE_LIST: {
            AstNode* operator = node->car;
            AstNode* arg = node->cdr;

            analyze_node(a, operator);

            if (operator && operator->type == NODE_ATOM && operator->token->type == TOKEN_IDENTIFIER) {
                const BuiltinInfo* info = get_builtin_info(operator->token->lexeme);

                if(info) {
                    int arg_count = count_args(arg);

                    if(info->min_arity != -1 && arg_count < info->min_arity){
                        report_error(operator->line, operator->column, 
                                     "Too few arguments to '%s'. Expected at least %d, got %d.",
                                     info->name, info->min_arity, arg_count);
                    }

                    if(info->max_arity != -1 && arg_count > info->max_arity){
                        report_error(operator->line, operator->column, 
                                     "Too many arguments to '%s'. Expected at most %d, got %d.",
                                     info->name, info->max_arity, arg_count);
                    }

                    if (info->arg_type != VAL_ANY){
                        AstNode* temp_arg = arg;
                        int position = 1;

                        while(temp_arg && temp_arg->type != NODE_NIL){
                            ValueType arg_type = get_node_type(temp_arg->car);
                            if(arg_type != VAL_ANY && arg_type != info->arg_type){
                                report_error(temp_arg->car->line, temp_arg->car->column, 
                                             "Argument %d to '%s' has incorrect type. Expected %s, got %s.",
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