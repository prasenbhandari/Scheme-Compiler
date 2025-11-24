#include "vm/instruction.h"
#include "utils/memory.h"
#include <stdio.h>

void init_bytecode(Bytecode* bc) {
    bc->instructions = NULL;
    bc->count = 0;
    bc->capacity = 0;
    bc->constants = NULL;
    bc->constant_count = 0;
    bc->constant_capacity = 0;
}

void free_bytecode(Bytecode* bc) {
    FREE_ARRAY(Instruction, bc->instructions, bc->capacity);
    FREE_ARRAY(Value, bc->constants, bc->constant_capacity);
    init_bytecode(bc);
}

int add_constant(Bytecode* bc, Value v) {
    if (bc->constant_capacity < bc->constant_count + 1) {
        int old_capacity = bc->constant_capacity;
        bc->constant_capacity = GROW_CAPACITY(old_capacity);
        bc->constants = GROW_ARRAY(Value, bc->constants, 
                                    old_capacity, bc->constant_capacity);
    }
    
    bc->constants[bc->constant_count] = v;
    return bc->constant_count++;
}

void patch_jump(Bytecode* bc, int jump_index, int target){
    bc->instructions[jump_index].operand = target;
}

void emit_instruction(Bytecode* bc, Opcode op, int operand) {
    if (bc->capacity < bc->count + 1) {
        int old_capacity = bc->capacity;
        bc->capacity = GROW_CAPACITY(old_capacity);
        bc->instructions = GROW_ARRAY(Instruction, bc->instructions,
                                       old_capacity, bc->capacity);
    }
    
    bc->instructions[bc->count].opcode = op;
    bc->instructions[bc->count].operand = operand;
    bc->count++;
}
