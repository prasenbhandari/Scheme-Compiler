#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "analyzer.h"
#include "symbol_table.h"
#include "error.h"


static const BuiltinInfo BUILTIN_TABLE[] = {
    // I/O procedures
    {"display", 1, 1, VAL_ANY},
    {"read", 0, 0, VAL_ANY},
    {"read-line", 0, 0, VAL_ANY},

    // Pair procedures
    {"cons", 2, 2, VAL_ANY},
    {"car", 1, 1, VAL_PAIR},
    {"cdr", 1, 1, VAL_PAIR},

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


static bool is_special_form(AstNode* operator) {
    if (!operator || operator->type != NODE_ATOM) return false;
    TokenType type = operator->token->type;
    
    return type == TOKEN_IF || 
           type == TOKEN_DEFINE || 
           type == TOKEN_LAMBDA ||
           type == TOKEN_LET ||
           type == TOKEN_LET_STAR ||
           type == TOKEN_LETREC ||
           type == TOKEN_LETREC_STAR ||
           type == TOKEN_BEGIN ||
           type == TOKEN_COND ||
           type == TOKEN_CASE ||
           type == TOKEN_QUOTE ||
           type == TOKEN_QUASIQUOTE ||
           type == TOKEN_UNQUOTE ||
           type == TOKEN_SET ||
           type == TOKEN_AND ||
           type == TOKEN_OR;
}


// Forward declarations for functions used below
static void analyze_node(Analyzer* a, AstNode* node);
static bool is_special_form(AstNode* operator);
static void analyze_special_form(Analyzer* a, AstNode* node);
static void analyze_builtin_call(Analyzer* a, AstNode* node);


static void analyze_quote(Analyzer* a, AstNode* node){
    int arg_count = count_args(node->cdr);

    if (arg_count != 1) {
        report_error(node->line, node->column,
            "'quote' requires exactly 1 argument, got %d", arg_count);
        return;
    }
}


static void analyze_if(Analyzer* a, AstNode* node){
    int arg_count = count_args(node->cdr);

    if (arg_count < 2 || arg_count > 3) {
        report_error(node->line, node->column,
            "'if' requires 2 or 3 arguments (condition then [else]), got %d", arg_count);
        return;
    }

    AstNode* condition = get_arg(node->cdr, 0);
    AstNode* then_branch = get_arg(node->cdr, 1);
    AstNode* else_branch = (arg_count == 3) ? get_arg(node->cdr, 2) : NULL;

    if (condition) analyze_node(a, condition);
    if (then_branch) analyze_node(a, then_branch);
    if (else_branch) analyze_node(a, else_branch);
}


static void analyze_cond(Analyzer* a, AstNode* node){
    AstNode* clauses = node->cdr;

    while (clauses && clauses->type != NODE_NIL){
        AstNode* clause = clauses->car;

        if(!clauses || clause->type != NODE_LIST || !clause->car){
            report_error(node->line, node->column,
                        "Invalid 'cond' clause: Expected a list with at least a condition");
            return;
        }

        AstNode* condition = clause->car;
        AstNode* result = clause->cdr;

        bool is_else = (condition->type == NODE_ATOM && condition->token->type == TOKEN_ELSE);

        // Analyze condition (skip for 'else' since it's not evaluated)
        if(!is_else) {           
            analyze_node(a, condition);
        } else {
            // 'else' is optional and must be last - check if there are more clauses
            if (clauses->cdr && clauses->cdr->type != NODE_NIL) {
                report_error(condition->line, condition->column,
                             "'else' clause must be the last clause in 'cond'");
                return;  // Stop processing after error
            }
        }

        // Analyze result expressions (for both regular and else clauses)
        while(result && result->type != NODE_NIL){
            analyze_node(a, result->car);
            result = result->cdr;
        }

        clauses = clauses->cdr;
    }

}


static void analyze_define(Analyzer* a, AstNode* node){
    AstNode* args = node->cdr;
    
    if (count_args(args) != 2){
        report_error(node->line, node->column,
                    "'define' requires 2 arguments (variable expression), got %d", count_args(args));
        return;
    }

    AstNode* var = args->car;
    AstNode* expr = args->cdr->car;

    if (var->type != NODE_ATOM || var->token->type != TOKEN_IDENTIFIER){
        report_error(var->line, var->column,
                    "'define' requires an identifier as the first argument");
        return;
    }

    analyze_node(a, expr);

    add_symbol(a->current_scope, var->token);
}


static void analyze_op(Analyzer* a, AstNode* node){
    AstNode* args = node->cdr;

    while(args && args->type != NODE_NIL){
        analyze_node(a, args->car);
        args = args->cdr;
    }
}


static void analyze_special_form(Analyzer* a, AstNode* node) {
    if (!node || !node->car) return;
    
    TokenType form = node->car->token->type;
    
    switch (form) {
        case TOKEN_IF:
            analyze_if(a, node);
            break;
        case TOKEN_COND:
            analyze_cond(a, node);
            break;
        case TOKEN_AND:
        case TOKEN_OR:
            analyze_op(a, node);
            break;
        case TOKEN_DEFINE:
            analyze_define(a, node);
            break;
        case TOKEN_QUOTE:
            analyze_quote(a, node);
            break;
        case TOKEN_LAMBDA:
            // Placeholder: analyze_lambda(a, node);
            report_error(node->line, node->column, 
                        "'lambda' special form not yet implemented in analyzer");
            break;
        case TOKEN_LET:
        case TOKEN_LET_STAR:
        case TOKEN_LETREC:
        case TOKEN_LETREC_STAR:
            // Placeholder: analyze_let(a, node);
            report_error(node->line, node->column, 
                        "'let' special form not yet implemented in analyzer");
            break;
        default:
            report_error(node->line, node->column, 
                        "Special form not yet implemented");
    }
}


static void analyze_builtin_call(Analyzer* a, AstNode* node) {
    AstNode* operator = node->car;
    AstNode* arg = node->cdr;

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
}


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

            if (is_special_form(operator)) {
                analyze_special_form(a, node);
                return;
            }

            analyze_builtin_call(a, node);

            while(arg && arg->type != NODE_NIL){
                analyze_node(a, arg->car);
                arg = arg->cdr;
            }
            break;
        }
    }
}