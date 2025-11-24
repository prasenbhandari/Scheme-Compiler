#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser.h"
#include "instruction.h"

// For REPL
Bytecode* compile(AstNode* node);

// For interpreting
void codegen_expr(Bytecode* bc, AstNode* node);
static void codegen_atom(Bytecode* bc, AstNode* ast);
static void codegen_list(Bytecode* bc, AstNode* ast);
static void codegen_builtin(Bytecode*bc, const char* op, AstNode* args);
static void codegen_if(Bytecode* bc, AstNode* ast);
static void codegen_cond(Bytecode* bc, AstNode* ast);
static void codegen_and(Bytecode* bc, AstNode* ast);
static void codegen_or(Bytecode* bc, AstNode* ast);
static void codegen_define(Bytecode* bc, AstNode* ast);

#endif // CODEGEN_H
