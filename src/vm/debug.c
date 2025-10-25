#include "vm/debug.h"
#include "vm/value.h"
#include <stdio.h>

static int simple_instruction(const char* name, int offset) {
    printf("%s\n", name);
    return offset + 1;
}

static int constant_instruction(const char* name, Bytecode* bc, int offset) {
    int constant_index = bc->instructions[offset].operand;
    printf("%-16s %4d '", name, constant_index);
    print_value(bc->constants[constant_index]);
    printf("'\n");
    return offset + 1;
}

void disassemble_instruction(Bytecode* bc, int offset) {
    printf("%04d ", offset);
    
    Instruction instr = bc->instructions[offset];
    
    switch (instr.opcode) {
        case OP_CONSTANT:
            constant_instruction("OP_CONSTANT", bc, offset);
            break;
        case OP_ADD:
            simple_instruction("OP_ADD", offset);
            break;
        case OP_DISPLAY:
            simple_instruction("OP_DISPLAY", offset);
            break;
        case OP_POP:
            simple_instruction("OP_POP", offset);
            break;
        case OP_HALT:
            simple_instruction("OP_HALT", offset);
            break;
        default:
            printf("Unknown opcode %d\n", instr.opcode);
            break;
    }
}

void disassemble_bytecode(Bytecode* bc, const char* name) {
    printf("== %s ==\n", name);
    
    for (int i = 0; i < bc->count; i++) {
        disassemble_instruction(bc, i);
    }
}
