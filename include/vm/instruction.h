#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "value.h"


typedef enum {
    OP_CONSTANT, // Load constant onto the stack
    OP_JUMP,          // Unconditional jump

    // Arithemetic
    OP_ADD,     // Add top two stack values
    OP_SUB,     // Subtract top two stack values
    OP_MUL,     // Multiply top two stack values
    OP_DIV,     // Divide top two stack values

    // Comparison
    OP_EQUAL,   // Check equality of top two stack values
    OP_GREATER, // Check if second top > top stack value
    OP_LESS,    // Check if second top < top stack value
    OP_NOT_EQUAL, // Check if second top != top stack value
    OP_GREATER_EQUAL, // Check if second top >= top stack value
    OP_LESS_EQUAL,    // Check if second top <= top stack value

    // If-else
    OP_JUMP_IF_FALSE, // Jump if top stack value is false

    // Variable management
    OP_DEFINE_GLOBAL,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,

    OP_DISPLAY, // Display top stack value
    OP_READ,    // Read input from user and push onto stack
    OP_READ_LINE, // Read a line of text from user
    OP_HALT,    // Stop execution
    OP_POP,     // Pop top value from the stack
    OP_JUMP_IF_TRUE_OR_POP, // Jump if true (keep value), else pop
    OP_JUMP_IF_FALSE_OR_POP, // Jump if false (keep value), else pop
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
void patch_jump(Bytecode* bc, int jump_index, int target);


#endif // INSTRUCTION_H
