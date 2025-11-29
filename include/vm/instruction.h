#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "value.h"
#include <stdint.h>


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

    // Pair operations
    OP_CONS,    // Create a new pair
    OP_CAR,     // Get the car of a pair
    OP_CDR,     // Get the cdr of a pair

    OP_DISPLAY, // Display top stack value
    OP_READ,    // Read input from user and push onto stack
    OP_READ_LINE, // Read a line of text from user
    OP_HALT,    // Stop execution
    OP_POP,     // Pop top value from the stack
    OP_NEWLINE, // Print a newline character
    OP_JUMP_IF_TRUE_OR_POP, // Jump if true (keep value), else pop
    OP_JUMP_IF_FALSE_OR_POP, // Jump if false (keep value), else pop

    // Functions and Closures
    OP_CLOSURE,
    OP_CALL,
    OP_RETURN,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
} Opcode;


typedef struct{
    uint8_t opcode;
    uint16_t operand;
} Instruction;


typedef struct Bytecode{
    Instruction* instructions;
    int32_t count;
    int32_t capacity;
    
    Value* constants;
    int32_t constant_count;
    int32_t constant_capacity;
} Bytecode;


void init_bytecode(Bytecode* bc);
void free_bytecode(Bytecode* bc);
int32_t add_constant(Bytecode* bc, Value v);
void emit_instruction(Bytecode* bc, Opcode op, int32_t operand); // Operand can be int32_t in arg, but stored as uint16_t
void patch_jump(Bytecode* bc, int32_t jump_index, int32_t target);


#endif // INSTRUCTION_H
