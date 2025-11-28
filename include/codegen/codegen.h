#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser.h"
#include "instruction.h"
#include "value.h"
#include <stdint.h>


typedef struct {
    const char* name;
    int depth;
} Local;


typedef struct Compiler {
    struct Compiler* enclosing;
    ObjFunction* function;

    Local locals[UINT8_MAX + 1];
    int local_count;
    int scope_depth;
} Compiler;


// For REPL
Bytecode* compile(AstNode* node);
Bytecode* compile_program(AstNode** nodes, int count);

// For interpreting
void codegen_expr(Compiler* compiler, AstNode* node);

#endif // CODEGEN_H
