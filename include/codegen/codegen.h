#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser.h"
#include "instruction.h"
#include "value.h"
#include <stdint.h>


typedef struct {
    const char* name;
    int depth;
    bool is_captured;
} Local;

typedef struct {
    uint8_t index;
    bool is_local;
} Upvalue;


typedef struct Compiler {
    struct Compiler* enclosing;
    ObjFunction* function;

    Local locals[UINT8_MAX + 1];
    uint8_t local_count;
    Upvalue upvalues[UINT8_MAX + 1];
    uint8_t scope_depth;
} Compiler;


// For REPL
Bytecode* compile(AstNode* node);
Bytecode* compile_program(AstNode** nodes, int count);

// For interpreting
void codegen_expr(Compiler* compiler, AstNode* node);

#endif // CODEGEN_H
