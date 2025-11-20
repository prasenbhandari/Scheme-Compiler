#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser.h"
#include "instruction.h"

// For REPL
Bytecode* compile(AstNode* node);

// For interpreting
void codegen_expr(Bytecode* bc, AstNode* node);

#endif // CODEGEN_H
