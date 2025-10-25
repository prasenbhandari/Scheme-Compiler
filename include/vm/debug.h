#ifndef DEBUG_H
#define DEBUG_H

#include "instruction.h"

void disassemble_bytecode(Bytecode* bc, const char* name);
void disassemble_instruction(Bytecode* bc, int offset);

#endif // DEBUG_H
