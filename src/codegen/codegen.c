#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "parser/parser.h"
#include "codegen/codegen.h"
#include "instruction.h"
#include "utils/error.h"
#include "utils/memory.h"


static void codegen_atom(Compiler* compiler, AstNode* ast);
static void codegen_list(Compiler* compiler, AstNode* ast);
static bool codegen_builtin(Compiler* compiler, const char* op, AstNode* args);
static void codegen_if(Compiler* compiler, AstNode* ast);
static void codegen_cond(Compiler* compiler, AstNode* ast);
static void codegen_and(Compiler* compiler, AstNode* ast);
static void codegen_or(Compiler* compiler, AstNode* ast);
static void codegen_define(Compiler* compiler, AstNode* ast);
static void codegen_quote(Compiler* compiler, AstNode* ast);
static void codegen_lambda(Compiler* compiler, AstNode* ast);


static void init_compiler(Compiler* compiler, Compiler* parent, int type){
    compiler->enclosing = parent;
    compiler->function = NULL;
    compiler->local_count = 0;
    compiler->scope_depth = 0;
}


static Bytecode* current_chunk(Compiler* compiler){
    return compiler->function->chunk;
}


static int resolve_local(Compiler* compiler, const char* name) {
    for (int i = compiler->local_count - 1; i >= 0; i--) {
        Local* local = &compiler->locals[i];
        if (strcmp(name, local->name) == 0) {
            return i;
        }
    }
    return -1;
}


static void codegen_atom(Compiler* compiler, AstNode* ast){
    Token* token = ast->token;
    Bytecode* bc = current_chunk(compiler);

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
            
            int local_idx = resolve_local(compiler, var_name);
            if (local_idx != -1) {
                emit_instruction(bc, OP_GET_LOCAL, local_idx);
            } else {
                int idx = add_constant(bc, STRING_VAL(var_name));
                emit_instruction(bc, OP_GET_GLOBAL, idx);
            }
            break;
        }

        default:
            report_error(ast->line, ast->column, 
                        "Code generation error: Unknown atom type");
            break;
    }
}


static void codegen_list(Compiler* compiler, AstNode* ast){
    AstNode* car = ast->car;

    // Handle Special Forms (Only if car is an ATOM)
    // We must check NODE_ATOM before checking token types to avoid crashing on lists
    if (car != NULL && car->type == NODE_ATOM) {
        TokenType type = car->token->type;

        if (type == TOKEN_IF) {
            codegen_if(compiler, ast);
            return;
        }

        if (type == TOKEN_COND) {
            codegen_cond(compiler, ast);
            return;
        }

        if (type == TOKEN_AND){
            codegen_and(compiler, ast);
            return;
        }

        if (type == TOKEN_OR) {
            codegen_or(compiler, ast);
            return;
        }

        if (type == TOKEN_DEFINE) {
            codegen_define(compiler, ast);
            return;
        }

        if (type == TOKEN_QUOTE) {
            codegen_quote(compiler, ast);
            return;
        }

        if (type == TOKEN_LAMBDA) {
            codegen_lambda(compiler, ast);
            return;
        }
    }

    // Prepare for Function Call
    // This handles both:
    //   (func-name arg1)         -> car is ATOM (identifier)
    //   ((lambda (x) x) arg1)    -> car is LIST (complex expression)
    
    AstNode* args = ast->cdr;
    
    // Builtin Optimization
    if (car != NULL && car->type == NODE_ATOM && car->token->type == TOKEN_IDENTIFIER) {
        if (codegen_builtin(compiler, car->token->lexeme, args)) {
            return;
        }
    }

    // Generic Function Call
    // Compile the function/operator
    codegen_expr(compiler, car);

    // Compile arguments (Pushes args onto stack)
    int arg_count = 0;
    AstNode* temp = args;
    while (temp && temp->type != NODE_NIL) {
        codegen_expr(compiler, temp->car);
        arg_count++;
        temp = temp->cdr;
    }
    
    // Emit CALL
    emit_instruction(current_chunk(compiler), OP_CALL, arg_count);
}


static bool codegen_builtin(Compiler* compiler, const char* op, AstNode* args) {
    Bytecode* bc = current_chunk(compiler);

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
                return true;
            }
        } else {
            // - and / require at least 1 argument
            if (arg_count == 0) {
                report_error(args ? args->line : -1, args ? args->column : -1,
                            "Operator '%s' requires at least 1 argument, got 0", op);
                return true; // Error handled
            }
        }
        
        codegen_expr(compiler, args->car);
        
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
            return true;
        }
        
        args = args->cdr;
        while (args && args->type != NODE_NIL) {
            codegen_expr(compiler, args->car);
            
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
        return true;
    }
    else if (strcmp(op, "<") == 0 || strcmp(op, ">") == 0 ||
             strcmp(op, "=") == 0 || strcmp(op, "<=") == 0 ||
             strcmp(op, ">=") == 0 || strcmp(op, "!=") == 0) {
        
        // Validate we have exactly 2 arguments
        if (args == NULL || args->type == NODE_NIL) {
            report_error(args ? args->line : -1, args ? args->column : -1,
                        "Operator '%s' requires 2 arguments, got 0", op);
            return true;
        }
        if (args->cdr == NULL || args->cdr->type == NODE_NIL) {
            report_error(args->line, args->column,
                        "Operator '%s' requires 2 arguments, got 1", op);
            return true;
        }
        
        codegen_expr(compiler, args->car);
        
        codegen_expr(compiler, args->cdr->car);
        
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
        return true;
    }
    else if (strcmp(op, "display") == 0) {
        // display takes one argument
        if (args == NULL || args->type == NODE_NIL) {
            report_error(args ? args->line : -1, args ? args->column : -1,
                        "Function 'display' requires 1 argument, got 0");
            return true;
        }
        
        // Compile the argument
        codegen_expr(compiler, args->car);
        
        // Emit DISPLAY instruction
        emit_instruction(bc, OP_DISPLAY, 0);
        return true;
    }
    else if (strcmp(op, "read") == 0) {
        if (args != NULL && args->type != NODE_NIL) {
            report_error(args->line, args->column,
                        "Function 'read' requires 0 arguments, got %d", 
                        (int)(args->cdr && args->cdr->type != NODE_NIL ? 2 : 1));
            return true;
        }
        
        emit_instruction(bc, OP_READ, 0);
        return true;
    }
    else if (strcmp(op, "read-line") == 0) {
        if (args != NULL && args->type != NODE_NIL) {
            report_error(args->line, args->column,
                        "Function 'read-line' requires 0 arguments, got %d", 
                        (int)(args->cdr && args->cdr->type != NODE_NIL ? 2 : 1));
            return true;
        }
        
        emit_instruction(bc, OP_READ_LINE, 0);
        return true;
    }
    else if (strcmp(op, "cons") == 0) {
        AstNode* arg1 = args;
        AstNode* arg2 = args->cdr;

        codegen_expr(compiler, arg1->car);
        codegen_expr(compiler, arg2->car);

        emit_instruction(bc, OP_CONS, 0);
        return true;
    }
    else if (strcmp(op, "car") == 0) {
        AstNode* arg1 = args;
        
        codegen_expr(compiler, arg1->car);
        emit_instruction(bc, OP_CAR, 0);
        return true;
    }
    else if (strcmp(op, "cdr") == 0) {
        AstNode* arg1 = args;
        
        codegen_expr(compiler, arg1->car);
        emit_instruction(bc, OP_CDR, 0);
        return true;
    }
    
    return false;
}


static Value ast_to_value(AstNode* node) {
    if (node == NULL) return NIL_VAL;

    switch (node->type) {
        case NODE_ATOM:
            switch (node->token->type) {
                case TOKEN_DEC:
                    return NUMBER_VAL(node->token->int_value);
                case TOKEN_REAL:
                    return NUMBER_VAL(node->token->real_value);
                case TOKEN_STR_LITERAL:
                    return STRING_VAL(node->token->lexeme);
                case TOKEN_TRUE:
                    return BOOL_VAL(true);
                case TOKEN_FALSE:
                    return BOOL_VAL(false);
                case TOKEN_IDENTIFIER:
                    // For now, symbols are just strings in our VM
                    return STRING_VAL(node->token->lexeme);
                default:
                    // Should not happen for valid AST
                    return NIL_VAL;
            }
        
        case NODE_LIST: {
            // Recursively convert list to pairs
            Value car_val = ast_to_value(node->car);
            Value cdr_val = ast_to_value(node->cdr);
            
            ObjPair* pair = malloc(sizeof(ObjPair));
            pair->car = car_val;
            pair->cdr = cdr_val;
            
            return PAIR_VAL(pair);
        }
        
        case NODE_NIL:
            return NIL_VAL;
            
        default:
            return NIL_VAL;
    }
}

static void codegen_quote(Compiler* compiler, AstNode* ast) {
    Bytecode* bc = current_chunk(compiler);
    AstNode* arg = ast->cdr->car;
    Value v = ast_to_value(arg);
    int idx = add_constant(bc, v);
    emit_instruction(bc, OP_CONSTANT, idx);
}


static void codegen_or(Compiler* compiler, AstNode* ast){
    Bytecode* bc = current_chunk(compiler);
    AstNode* args = ast->cdr;

    if(args->type == NODE_NIL){
        int idx = add_constant(bc, BOOL_VAL(false));
        emit_instruction(bc, OP_CONSTANT, idx);
        return;
    }

    int last_jump = -1;

    while(args->cdr && args->cdr->type != NODE_NIL){
        codegen_expr(compiler, args->car);

        emit_instruction(bc, OP_JUMP_IF_TRUE_OR_POP, last_jump);
        last_jump = bc->count - 1;

        args = args->cdr;
    }

    codegen_expr(compiler, args->car);

    while (last_jump != -1) {
        int next_jump = bc->instructions[last_jump].operand;
        patch_jump(bc, last_jump, bc->count);
        last_jump = next_jump;
    }
}


static void codegen_and(Compiler* compiler, AstNode* ast){
    Bytecode* bc = current_chunk(compiler);
    AstNode* args = ast->cdr;

    if(args->type == NODE_NIL){
        int idx = add_constant(bc, BOOL_VAL(true));
        emit_instruction(bc, OP_CONSTANT, idx);
        return;
    }
    
    int last_jump = -1;

    while(args->cdr && args->cdr->type != NODE_NIL){
        AstNode* arg = args->car;

        codegen_expr(compiler, arg);

        // Emit jump, pointing to the PREVIOUS jump in the chain
        emit_instruction(bc, OP_JUMP_IF_FALSE_OR_POP, last_jump);
        
        // Update head of chain to this instruction
        last_jump = bc->count - 1;

        args = args->cdr;

    }    

    codegen_expr(compiler, args->car);

    // Walk the chain and patch everything to HERE
    while (last_jump != -1) {
        int next_jump = bc->instructions[last_jump].operand;
        patch_jump(bc, last_jump, bc->count);
        last_jump = next_jump;
    }
}


static void codegen_cond(Compiler* compiler, AstNode* ast){
    Bytecode* bc = current_chunk(compiler);
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
                    codegen_expr(compiler, body->car);
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

        codegen_expr(compiler, condition);

        int jump_false_index = bc->count;
        emit_instruction(bc, OP_JUMP_IF_FALSE, 0);

        if (body && body->type != NODE_NIL){
            while(body && body->type != NODE_NIL){
                codegen_expr(compiler, body->car);
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


static void codegen_if(Compiler* compiler, AstNode* ast){
    Bytecode* bc = current_chunk(compiler);
    AstNode* condition = get_arg(ast->cdr, 0);
    AstNode* then_branch = get_arg(ast->cdr, 1);
    AstNode* else_branch = get_arg(ast->cdr, 2);

    if(!condition || !then_branch){
        report_error(ast->line, ast->column,
                    "'if' expression requires at least condition and then-branch");
        return;
    }

    codegen_expr(compiler, condition);

    int jump_if_false_index = bc->count;;

    emit_instruction(bc, OP_JUMP_IF_FALSE, 0);

    codegen_expr(compiler, then_branch);

    int jump_over_else_index = bc->count;
    emit_instruction(bc, OP_JUMP, 0);

    int else_start_index = bc->count;
    patch_jump(bc, jump_if_false_index, else_start_index);

    if(else_branch){
        codegen_expr(compiler, else_branch);
    } else {
        int idx = add_constant(bc, NIL_VAL);
        emit_instruction(bc, OP_CONSTANT, idx);
    }

    int after_else_index = bc->count;
    patch_jump(bc, jump_over_else_index, after_else_index);
}


void codegen_define(Compiler* compiler, AstNode* ast){
    Bytecode* bc = current_chunk(compiler);
    AstNode* args = ast->cdr;

    const char* var_name = args->car->token->lexeme;

    AstNode* value_node = args->cdr->car;

    codegen_expr(compiler, value_node);

    int name_idx = add_constant(bc, STRING_VAL(var_name));

    emit_instruction(bc, OP_DEFINE_GLOBAL, name_idx);
}

static void codegen_lambda(Compiler* current, AstNode* ast) {
    // Create a new Compiler for this function
    Compiler compiler;
    init_compiler(&compiler, current, 0);
    
    // Create the function object
    compiler.function = malloc(sizeof(ObjFunction));
    compiler.function->arity = 0;
    compiler.function->upvalue_count = 0;
    compiler.function->name = NULL; // Anonymous
    compiler.function->chunk = malloc(sizeof(Bytecode));
    init_bytecode(compiler.function->chunk);
    
    // Parse arguments
    AstNode* args = ast->cdr->car;
    while (args && args->type != NODE_NIL) {
        compiler.function->arity++;
        if (compiler.local_count < UINT8_MAX) {
            Local* local = &compiler.locals[compiler.local_count++];
            local->name = args->car->token->lexeme;
            local->depth = 1;
        }
        args = args->cdr;
    }
    
    // Compile Body
    AstNode* body = ast->cdr->cdr;
    while (body && body->type != NODE_NIL) {
        codegen_expr(&compiler, body->car);
        // If there are multiple expressions, pop the result of the previous ones
        if (body->cdr && body->cdr->type != NODE_NIL) {
            emit_instruction(current_chunk(&compiler), OP_POP, 0);
        }
        body = body->cdr;
    }
    
    // Emit Return
    emit_instruction(current_chunk(&compiler), OP_RETURN, 0);
    
    // Wrap in Closure in the PARENT chunk
    int constant = add_constant(current_chunk(current), FUNCTION_VAL(compiler.function));
    emit_instruction(current_chunk(current), OP_CLOSURE, constant);
}


void codegen_expr(Compiler* compiler, AstNode* ast){
    if (ast == NULL || ast->type == NODE_NIL) {
        return;
    }

    switch (ast->type){
        case NODE_ATOM:
            codegen_atom(compiler, ast);
            break;
        
        case NODE_LIST:
            codegen_list(compiler, ast);
            break;
        
        default:
            report_error(ast->line, ast->column,
                        "Code generation error: Unknown node type");
            break;
    }
}


Bytecode* compile(AstNode* ast) {
    Compiler compiler;
    init_compiler(&compiler, NULL, 0);
    
    // Create top-level script function
    compiler.function = malloc(sizeof(ObjFunction));
    compiler.function->arity = 0;
    compiler.function->upvalue_count = 0;
    compiler.function->name = NULL;
    compiler.function->chunk = malloc(sizeof(Bytecode));
    init_bytecode(compiler.function->chunk);

    codegen_expr(&compiler, ast);

    emit_instruction(current_chunk(&compiler), OP_HALT, 0);

    return compiler.function->chunk;
}

Bytecode* compile_program(AstNode** nodes, int count) {
    Compiler compiler;
    init_compiler(&compiler, NULL, 0);
    
    // Create top-level script function
    compiler.function = malloc(sizeof(ObjFunction));
    compiler.function->arity = 0;
    compiler.function->upvalue_count = 0;
    compiler.function->name = NULL;
    compiler.function->chunk = malloc(sizeof(Bytecode));
    init_bytecode(compiler.function->chunk);

    for (int i = 0; i < count; i++) {
        codegen_expr(&compiler, nodes[i]);
        // Pop the result of each expression after executing it
        if (i < count - 1) {
            emit_instruction(current_chunk(&compiler), OP_POP, 0);
        }
    }

    emit_instruction(current_chunk(&compiler), OP_HALT, 0);

    return compiler.function->chunk;
}
