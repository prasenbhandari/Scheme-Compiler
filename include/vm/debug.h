#ifndef DEBUG_H
#define DEBUG_H

#include "instruction.h"
#include <stdint.h>

void disassemble_bytecode(Bytecode* bc, const char* name);
void disassemble_instruction(Bytecode* bc, int32_t offset);

#endif // DEBUG_H
