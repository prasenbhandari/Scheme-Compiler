#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "parser/parser.h"
#include "codegen/codegen.h"
#include "instruction.h"
#include "utils/error.h"
#include "utils/memory.h"


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

        case TOKEN_IDENTIFIER: {
            const char* var_name = token->lexeme;
            int idx = add_constant(bc, STRING_VAL(var_name));
            emit_instruction(bc, OP_GET_GLOBAL, idx);
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

    if (car->token->type == TOKEN_DEFINE) {
        codegen_define(bc, ast);
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
    // Handle variadic arithmetic operators
    if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0 || 
        strcmp(op, "*") == 0 || strcmp(op, "/") == 0) {
        
        int arg_count = 0;
        AstNode* temp = args;
        while (temp && temp->type != NODE_NIL) {
            arg_count++;
            temp = temp->cdr;
        }
        
        if (strcmp(op, "+") == 0 || strcmp(op, "*") == 0) {
            // + and * can have 0 or more arguments
            if (arg_count == 0) {
                // (+) => 0, (*) => 1
                int idx = add_constant(bc, NUMBER_VAL(strcmp(op, "+") == 0 ? 0.0 : 1.0));
                emit_instruction(bc, OP_CONSTANT, idx);
                return;
            }
        } else {
            // - and / require at least 1 argument
            if (arg_count == 0) {
                report_error(args ? args->line : -1, args ? args->column : -1,
                            "Operator '%s' requires at least 1 argument, got 0", op);
                return;
            }
        }
        
        codegen_expr(bc, args->car);
        
        // If only one argument for - or /, apply unary operation
        if (arg_count == 1 && (strcmp(op, "-") == 0 || strcmp(op, "/") == 0)) {
            if (strcmp(op, "-") == 0) {
                // (- x) => 0 - x
                int zero_idx = add_constant(bc, NUMBER_VAL(0.0));
                emit_instruction(bc, OP_CONSTANT, zero_idx);
                emit_instruction(bc, OP_SUB, 0);
            } else {
                // (/ x) => 1 / x
                int one_idx = add_constant(bc, NUMBER_VAL(1.0));
                emit_instruction(bc, OP_CONSTANT, one_idx);
                emit_instruction(bc, OP_DIV, 0);
            }
            return;
        }
        
        args = args->cdr;
        while (args && args->type != NODE_NIL) {
            codegen_expr(bc, args->car);
            
            if (strcmp(op, "+") == 0) {
                emit_instruction(bc, OP_ADD, 0);
            } else if (strcmp(op, "-") == 0) {
                emit_instruction(bc, OP_SUB, 0);
            } else if (strcmp(op, "*") == 0) {
                emit_instruction(bc, OP_MUL, 0);
            } else if (strcmp(op, "/") == 0) {
                emit_instruction(bc, OP_DIV, 0);
            }
            
            args = args->cdr;
        }
    }
    else if (strcmp(op, "<") == 0 || strcmp(op, ">") == 0 ||
             strcmp(op, "=") == 0 || strcmp(op, "<=") == 0 ||
             strcmp(op, ">=") == 0 || strcmp(op, "!=") == 0) {
        
        // Validate we have exactly 2 arguments
        if (args == NULL || args->type == NODE_NIL) {
            report_error(args ? args->line : -1, args ? args->column : -1,
                        "Operator '%s' requires 2 arguments, got 0", op);
            return;
        }
        if (args->cdr == NULL || args->cdr->type == NODE_NIL) {
            report_error(args->line, args->column,
                        "Operator '%s' requires 2 arguments, got 1", op);
            return;
        }
        
        codegen_expr(bc, args->car);
        
        codegen_expr(bc, args->cdr->car);
        
        // Emit the appropriate instruction based on operator
        if (strcmp(op, "<") == 0) {
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
    else if (strcmp(op, "read") == 0) {
        if (args != NULL && args->type != NODE_NIL) {
            report_error(args->line, args->column,
                        "Function 'read' requires 0 arguments, got %d", 
                        (int)(args->cdr && args->cdr->type != NODE_NIL ? 2 : 1));
            return;
        }
        
        emit_instruction(bc, OP_READ, 0);
    }
    else if (strcmp(op, "read-line") == 0) {
        if (args != NULL && args->type != NODE_NIL) {
            report_error(args->line, args->column,
                        "Function 'read-line' requires 0 arguments, got %d", 
                        (int)(args->cdr && args->cdr->type != NODE_NIL ? 2 : 1));
            return;
        }
        
        emit_instruction(bc, OP_READ_LINE, 0);
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


void codegen_define(Bytecode* bc, AstNode* ast){
    AstNode* args = ast->cdr;

    const char* var_name = args->car->token->lexeme;

    AstNode* value_node = args->cdr->car;

    codegen_expr(bc, value_node);

    int name_idx = add_constant(bc, STRING_VAL(var_name));

    emit_instruction(bc, OP_DEFINE_GLOBAL, name_idx);
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
