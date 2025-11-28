#ifndef VM_H
#define VM_H

#include "instruction.h"
#include "value.h"
#include "table.h"
#include <stdbool.h>
#include <stdint.h>

#define STACK_MAX 256
#define FRAMES_MAX 64

typedef struct {
    ObjClosure* closure;
    Bytecode* parent_code;  // Parent function's bytecode
    uint32_t ip;  // Instruction index in parent bytecode
    Value* slots;
} CallFrame;

typedef struct {
    CallFrame frames[FRAMES_MAX];
    int32_t frame_count;

    Bytecode* code;
    uint32_t ip;
    Value stack[STACK_MAX];
    int32_t stack_top;
    bool trace_execution;  // Flag to enable/disable instruction tracing
    Table globals;
} VM;

void init_vm(VM* vm);
void free_vm(VM* vm);
void vm_execute(VM* vm, Bytecode* bc);

void push(VM* vm, Value v);
Value pop(VM* vm);
Value peek_stack(VM* vm, int32_t distance);

#endif // VM_H
