#include "vm/debug.h"
#include "vm/value.h"
#include <stdio.h>

static int32_t simple_instruction(const char* name, int32_t offset) {
    printf("%s\n", name);
    return offset + 1;
}

static int32_t constant_instruction(const char* name, Bytecode* bc, int32_t offset) {
    int32_t constant_index = bc->instructions[offset].operand;
    printf("%-16s %4d '", name, constant_index);
    print_value(bc->constants[constant_index]);
    printf("'\n");
    return offset + 1;
}

static int32_t jump_instruction(const char* name, int32_t offset, int32_t operand) {
    printf("%-16s %4d\n", name, operand);
    return offset + 1;
}

void disassemble_instruction(Bytecode* bc, int32_t offset) {
    printf("%04d ", offset);
    
    Instruction instr = bc->instructions[offset];
    
    switch (instr.opcode) {
        case OP_CONSTANT:
            constant_instruction("OP_CONSTANT", bc, offset);
            break;
        case OP_JUMP:
            jump_instruction("OP_JUMP", offset, instr.operand);
            break;
        case OP_JUMP_IF_FALSE:
            jump_instruction("OP_JUMP_IF_FALSE", offset, instr.operand);
            break;
        case OP_ADD:
            simple_instruction("OP_ADD", offset);
            break;
        case OP_SUB:
            simple_instruction("OP_SUB", offset);
            break;
        case OP_MUL:
            simple_instruction("OP_MUL", offset);
            break;
        case OP_DIV:
            simple_instruction("OP_DIV", offset);
            break;
        case OP_LESS:
            simple_instruction("OP_LESS", offset);
            break;
        case OP_GREATER:
            simple_instruction("OP_GREATER", offset);
            break;
        case OP_EQUAL:
            simple_instruction("OP_EQUAL", offset);
            break;
        case OP_LESS_EQUAL:
            simple_instruction("OP_LESS_EQUAL", offset);
            break;
        case OP_GREATER_EQUAL:
            simple_instruction("OP_GREATER_EQUAL", offset);
            break;
        case OP_NOT_EQUAL:
            simple_instruction("OP_NOT_EQUAL", offset);
            break;
        case OP_DISPLAY:
            simple_instruction("OP_DISPLAY", offset);
            break;
        case OP_NEWLINE:
            simple_instruction("OP_NEWLINE", offset);
            break;
        case OP_POP:
            simple_instruction("OP_POP", offset);
            break;
        case OP_HALT:
            simple_instruction("OP_HALT", offset);
            break;
        case OP_JUMP_IF_TRUE_OR_POP:
            jump_instruction("OP_JUMP_IF_TRUE_OR_POP", offset, instr.operand);
            break;
        case OP_JUMP_IF_FALSE_OR_POP:
            jump_instruction("OP_JUMP_IF_FALSE_OR_POP", offset, instr.operand);
            break;
        case OP_DEFINE_GLOBAL:
            constant_instruction("OP_DEFINE_GLOBAL", bc, offset);
            break;
        case OP_GET_GLOBAL:
            constant_instruction("OP_GET_GLOBAL", bc, offset);
            break;
        case OP_SET_GLOBAL:
            constant_instruction("OP_SET_GLOBAL", bc, offset);
            break;
        case OP_CONS:
            simple_instruction("OP_CONS", offset);
            break;
        case OP_CAR:
            simple_instruction("OP_CAR", offset);
            break;
        case OP_CDR:
            simple_instruction("OP_CDR", offset);
            break;
        case OP_CLOSURE:
            constant_instruction("OP_CLOSURE", bc, offset);
            break;
        case OP_CALL:
            jump_instruction("OP_CALL", offset, instr.operand);
            break;
        case OP_RETURN:
            simple_instruction("OP_RETURN", offset);
            break;
        case OP_GET_LOCAL:
            simple_instruction("OP_GET_LOCAL", offset);
            break;
        case OP_SET_LOCAL:
            simple_instruction("OP_SET_LOCAL", offset);
            break;
        default:
            printf("Unknown opcode %d\n", instr.opcode);
            break;
    }
}

void disassemble_bytecode(Bytecode* bc, const char* name) {
    printf("== %s ==\n", name);
    
    for (int32_t i = 0; i < bc->count; i++) {
        disassemble_instruction(bc, i);
    }
}
