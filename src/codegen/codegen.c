#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "codegen/codegen.h"
#include "instruction.h"
#include "utils/error.h"


static void codegen_atom(Bytecode* bc, AstNode* ast);
static void codegen_list(Bytecode* bc, AstNode* ast);
static void codegen_builtin(Bytecode*bc, const char* op, AstNode* args);

static void codegen_atom(Bytecode* bc, AstNode* ast){
    Token* token = ast->token;

    switch(token->type){
        case TOKEN_DEC: {
            int idx = add_constant(bc, NUMBER_VAL(token->int_value));
            emit_instruction(bc, OP_CONSTANT, idx);
            break;
        }

        case TOKEN_REAL: {
            int idx = add_constant(bc, NUMBER_VAL(token->real_value));
            emit_instruction(bc, OP_CONSTANT, idx);
            break;
        }

        case TOKEN_STR_LITERAL: {
            int idx = add_constant(bc, STRING_VAL(token->lexeme));
            emit_instruction(bc, OP_CONSTANT, idx);
            break;
        }

        default:
            report_error(ast->line, ast->column, 
                        "Code generation error: Unknown atom type");
            break;
    }
}


static void codegen_list(Bytecode* bc, AstNode* ast){
    AstNode* car = ast->car;

    if(car == NULL || car->type != NODE_ATOM){
        report_error(ast->line, ast->column, 
                    "Invalid function call: Expected identifier");
        return;
    }

    if(car->token->type != TOKEN_IDENTIFIER){
        report_error(car->line, car->column, 
                    "Invalid function name: Expected identifier, got '%s'", 
                    car->token->lexeme ? car->token->lexeme : "(unknown)");
        return;
    }

    const char* operator = car->token->lexeme;
    AstNode* args = ast->cdr;

    codegen_builtin(bc, operator, args);
}


static void codegen_builtin(Bytecode* bc, const char* op, AstNode* args) {
    // Handle binary arithmetic and comparison operators
    if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0 || 
        strcmp(op, "*") == 0 || strcmp(op, "/") == 0 ||
        strcmp(op, "<") == 0 || strcmp(op, ">") == 0 ||
        strcmp(op, "=") == 0 || strcmp(op, "<=") == 0 ||
        strcmp(op, ">=") == 0 || strcmp(op, "!=") == 0) {
        
        // Validate we have at least 2 arguments
        if (args == NULL || args->type == NODE_NIL) {
            report_error(args ? args->line : -1, args ? args->column : -1,
                        "Operator '%s' requires at least 2 arguments, got 0", op);
            return;
        }
        if (args->cdr == NULL || args->cdr->type == NODE_NIL) {
            report_error(args->line, args->column,
                        "Operator '%s' requires at least 2 arguments, got 1", op);
            return;
        }
        
        // Compile first argument (handles both atoms and nested lists automatically)
        codegen_expr(bc, args->car);
        
        // Compile second argument
        codegen_expr(bc, args->cdr->car);
        
        // Emit the appropriate instruction based on operator
        if (strcmp(op, "+") == 0) {
            emit_instruction(bc, OP_ADD, 0);
        } else if (strcmp(op, "-") == 0) {
            emit_instruction(bc, OP_SUB, 0);
        } else if (strcmp(op, "*") == 0) {
            emit_instruction(bc, OP_MUL, 0);
        } else if (strcmp(op, "/") == 0) {
            emit_instruction(bc, OP_DIV, 0);
        } else if (strcmp(op, "<") == 0) {
            emit_instruction(bc, OP_LESS, 0);
        } else if (strcmp(op, ">") == 0) {
            emit_instruction(bc, OP_GREATER, 0);
        } else if (strcmp(op, "=") == 0) {
            emit_instruction(bc, OP_EQUAL, 0);
        } else if (strcmp(op, "<=") == 0) {
            emit_instruction(bc, OP_LESS_EQUAL, 0);
        } else if (strcmp(op, ">=") == 0) {
            emit_instruction(bc, OP_GREATER_EQUAL, 0);
        } else if (strcmp(op, "!=") == 0) {
            emit_instruction(bc, OP_NOT_EQUAL, 0);
        }
    }
    else if (strcmp(op, "display") == 0) {
        // display takes one argument
        if (args == NULL || args->type == NODE_NIL) {
            report_error(args ? args->line : -1, args ? args->column : -1,
                        "Function 'display' requires 1 argument, got 0");
            return;
        }
        
        // Compile the argument
        codegen_expr(bc, args->car);
        
        // Emit DISPLAY instruction
        emit_instruction(bc, OP_DISPLAY, 0);
    }
    else {
        // Unknown built-in - get position from first arg if available
        int line = args ? args->line : -1;
        int col = args ? args->column : -1;
        report_error(line, col, "Unknown function: '%s'", op);
    }
}


void codegen_expr(Bytecode* bc, AstNode* ast){
    if (ast == NULL || ast->type == NODE_NIL) {
        return;
    }

    switch (ast->type){
        case NODE_ATOM:
            codegen_atom(bc, ast);
            break;
        
        case NODE_LIST:
            codegen_list(bc, ast);
            break;
        
        default:
            report_error(ast->line, ast->column,
                        "Code generation error: Unknown node type");
            break;
    }
}


Bytecode* compile(AstNode* ast) {
    Bytecode* bc = malloc(sizeof(Bytecode));
    init_bytecode(bc);

    codegen_expr(bc, ast);

    emit_instruction(bc, OP_HALT, 0);

    return bc;
}
