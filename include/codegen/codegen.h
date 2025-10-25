#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser.h"
#include "instruction.h"

Bytecode* compile(AstNode* node);

#endif // CODEGEN_H
