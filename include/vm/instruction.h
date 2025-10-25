#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "value.h"


typedef enum {
    OP_CONSTANT, // Load constant onto the stack
    OP_ADD,     // Add top two stack values
    OP_DISPLAY, // Display top stack value
    OP_HALT,    // Stop execution
    OP_POP,     // Pop top value from the stack
} Opcode;


typedef struct{
    Opcode opcode;
    int operand;
} Instruction;


typedef struct {
    Instruction* instructions;
    int count;
    int capacity;
    
    Value* constants;
    int constant_count;
    int constant_capacity;
} Bytecode;


void init_bytecode(Bytecode* bc);
void free_bytecode(Bytecode* bc);
int add_constant(Bytecode* bc, Value v);
void emit_instruction(Bytecode* bc, Opcode op, int operand);


#endif // INSTRUCTION_H
