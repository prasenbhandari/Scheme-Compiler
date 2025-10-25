#include <string.h>
#include <stdlib.h>

#include "codegen/codegen.h"
#include "instruction.h"


static void codegen_expr(Bytecode* bc, AstNode* ast);
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
            fprintf(stderr, "Unknown atom type");
            break;
    }
}


static void codegen_list(Bytecode* bc, AstNode* ast){
    AstNode* car = ast->car;

    if(car == NULL || car->type != NODE_ATOM){
        fprintf(stderr, "Invalid function call\n");
        return;
    }

    if(car->token->type != TOKEN_IDENTIFIER){
        fprintf(stderr, "Invalid function name\n");
        return;
    }

    const char* operator = car->token->lexeme;
    AstNode* args = ast->cdr;

    codegen_builtin(bc, operator, args);
}


static void codegen_builtin(Bytecode* bc, const char* op, AstNode* args) {
    if (strcmp(op, "+") == 0) {
        // args->car is first argument
        // args->cdr->car is second argument
        
        if (args == NULL || args->type == NODE_NIL) {
            fprintf(stderr, "Error: + requires at least 2 arguments\n");
            return;
        }
        
        // Compile first argument
        codegen_expr(bc, args->car);
        
        if (args->cdr == NULL || args->cdr->type == NODE_NIL) {
            fprintf(stderr, "Error: + requires at least 2 arguments\n");
            return;
        }
        
        // Compile second argument
        codegen_expr(bc, args->cdr->car);
        
        // Emit ADD instruction
        emit_instruction(bc, OP_ADD, 0);
    }
    else if (strcmp(op, "display") == 0) {
        // display takes one argument
        if (args == NULL || args->type == NODE_NIL) {
            fprintf(stderr, "Error: display requires 1 argument\n");
            return;
        }
        
        // Compile the argument
        codegen_expr(bc, args->car);
        
        // Emit DISPLAY instruction
        emit_instruction(bc, OP_DISPLAY, 0);
    }
    else {
        fprintf(stderr, "Error: Unknown built-in: %s\n", op);
    }
}





static void codegen_expr(Bytecode* bc, AstNode* ast){
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
            fprintf(stderr, "Unknown node type\n");
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
