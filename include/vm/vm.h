#ifndef VM_H
#define VM_H

#include "instruction.h"
#include "value.h"
#include <stdbool.h>

#define STACK_MAX 256

typedef struct {
    Bytecode* code;
    int ip;
    Value stack[STACK_MAX];
    int stack_top;
    bool trace_execution;  // Flag to enable/disable instruction tracing
} VM;

void init_vm(VM* vm);
void free_vm(VM* vm);
void vm_execute(VM* vm, Bytecode* bc);

void push(VM* vm, Value v);
Value pop(VM* vm);
Value peek_stack(VM* vm, int distance);

#endif // VM_H
