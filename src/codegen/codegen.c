#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "parser/parser.h"
#include "codegen/codegen.h"
#include "instruction.h"
#include "utils/error.h"
#include "vm/memory.h"


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

        case TOKEN_TRUE: {
            int idx = add_constant(bc , BOOL_VAL(true));
            emit_instruction(bc, OP_CONSTANT, idx);
            break;
        }

        case TOKEN_FALSE: {
            int idx = add_constant(bc, BOOL_VAL(false));
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

    if (car->token->type == TOKEN_IF) {
        codegen_if(bc, ast);
        return;
    }

    if (car->token->type == TOKEN_COND) {
        codegen_cond(bc, ast);
        return;
    }

    if(car->token->type == TOKEN_AND){
        codegen_and(bc, ast);
        return;
    }

    if (car->token->type == TOKEN_OR) {
        codegen_or(bc, ast);
        return;
    }

    if(car->token->type != TOKEN_IDENTIFIER){
        report_error(car->line, car->column, 
                    "Invalid function name: Expected identifier or special form, got '%s'", 
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


static void codegen_or(Bytecode* bc, AstNode* ast){
    AstNode* args = ast->cdr;

    if(args->type == NODE_NIL){
        int idx = add_constant(bc, BOOL_VAL(false));
        emit_instruction(bc, OP_CONSTANT, idx);
        return;
    }

    int last_jump = -1;

    while(args->cdr && args->cdr->type != NODE_NIL){
        codegen_expr(bc, args->car);

        emit_instruction(bc, OP_JUMP_IF_TRUE_OR_POP, last_jump);
        last_jump = bc->count - 1;

        args = args->cdr;
    }

    codegen_expr(bc, args->car);

    while (last_jump != -1) {
        int next_jump = bc->instructions[last_jump].operand;
        patch_jump(bc, last_jump, bc->count);
        last_jump = next_jump;
    }
}


static void codegen_and(Bytecode* bc, AstNode* ast){
    AstNode* args = ast->cdr;

    if(args->type == NODE_NIL){
        int idx = add_constant(bc, BOOL_VAL(true));
        emit_instruction(bc, OP_CONSTANT, idx);
        return;
    }
    
    int last_jump = -1;

    while(args->cdr && args->cdr->type != NODE_NIL){
        AstNode* arg = args->car;

        codegen_expr(bc, arg);

        // Emit jump, pointing to the PREVIOUS jump in the chain
        emit_instruction(bc, OP_JUMP_IF_FALSE_OR_POP, last_jump);
        
        // Update head of chain to this instruction
        last_jump = bc->count - 1;

        args = args->cdr;

    }    

    codegen_expr(bc, args->car);

    // Walk the chain and patch everything to HERE
    while (last_jump != -1) {
        int next_jump = bc->instructions[last_jump].operand;
        patch_jump(bc, last_jump, bc->count);
        last_jump = next_jump;
    }
}


static void codegen_cond(Bytecode* bc, AstNode* ast){
    AstNode* clauses = ast->cdr;

    int last_exit_jump = -1;
    bool has_else = false;

    while(clauses && clauses->type != NODE_NIL){
        AstNode* clause = clauses->car;
        AstNode* condition = clause->car;
        AstNode* body = clause->cdr;

        bool is_else = (condition->type == NODE_ATOM && condition->token->type == TOKEN_ELSE);

        if(is_else){
            has_else = true;
            if (body && body->type != NODE_NIL){
                while(body && body->type != NODE_NIL){
                    codegen_expr(bc, body->car);
                    if(body->cdr && body->cdr->type != NODE_NIL){
                        emit_instruction(bc, OP_POP, 0);
                    }
                    body = body->cdr;
                }
            } else {
                int idx = add_constant(bc, NIL_VAL);
                emit_instruction(bc, OP_CONSTANT, idx);
            }
            break;
        } 

        codegen_expr(bc, condition);

        int jump_false_index = bc->count;
        emit_instruction(bc, OP_JUMP_IF_FALSE, 0);

        if (body && body->type != NODE_NIL){
            while(body && body->type != NODE_NIL){
                codegen_expr(bc, body->car);
                if(body->cdr && body->cdr->type != NODE_NIL){
                    emit_instruction(bc, OP_POP, 0);
                }
                body = body->cdr;
            }
        } else {
            int idx = add_constant(bc, NIL_VAL);
            emit_instruction(bc, OP_CONSTANT, idx);
        }

        emit_instruction(bc, OP_JUMP, last_exit_jump);
        last_exit_jump = bc->count - 1;

        patch_jump(bc, jump_false_index, bc->count);

        clauses = clauses->cdr;

    }

    if (!has_else) {
        int idx = add_constant(bc, NIL_VAL);
        emit_instruction(bc, OP_CONSTANT, idx);
    }

    while (last_exit_jump != -1) {
        int next_jump = bc->instructions[last_exit_jump].operand;
        patch_jump(bc, last_exit_jump, bc->count);
        last_exit_jump = next_jump;
    }
}


static void codegen_if(Bytecode* bc, AstNode* ast){
    AstNode* condition = get_arg(ast->cdr, 0);
    AstNode* then_branch = get_arg(ast->cdr, 1);
    AstNode* else_branch = get_arg(ast->cdr, 2);

    if(!condition || !then_branch){
        report_error(ast->line, ast->column,
                    "'if' expression requires at least condition and then-branch");
        return;
    }

    codegen_expr(bc, condition);

    int jump_if_false_index = bc->count;;

    emit_instruction(bc, OP_JUMP_IF_FALSE, 0);

    codegen_expr(bc, then_branch);

    int jump_over_else_index = bc->count;
    emit_instruction(bc, OP_JUMP, 0);

    int else_start_index = bc->count;
    patch_jump(bc, jump_if_false_index, else_start_index);

    if(else_branch){
        codegen_expr(bc, else_branch);
    }

    int after_else_index = bc->count;
    patch_jump(bc, jump_over_else_index, after_else_index);
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
