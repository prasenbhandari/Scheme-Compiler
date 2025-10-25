#include "vm/vm.h"
#include "vm/debug.h"
#include <stdio.h>
#include <stdlib.h>

void init_vm(VM* vm) {
    vm->stack_top = 0;
    vm->ip = 0;
    vm->code = NULL;
    vm->trace_execution = false;  // Tracing disabled by default
}

void free_vm(VM* vm) {
    // Nothing to free for now
}

void push(VM* vm, Value v) {
    if (vm->stack_top >= STACK_MAX) {
        fprintf(stderr, "Stack overflow!\n");
        exit(1);
    }
    vm->stack[vm->stack_top++] = v;
}

Value pop(VM* vm) {
    if (vm->stack_top <= 0) {
        fprintf(stderr, "Stack underflow!\n");
        exit(1);
    }
    return vm->stack[--vm->stack_top];
}

Value peek_stack(VM* vm, int distance) {
    return vm->stack[vm->stack_top - 1 - distance];
}

void vm_execute(VM* vm, Bytecode* bc) {
    vm->code = bc;
    vm->ip = 0;
    
    while (vm->ip < bc->count) {
        // Trace execution if enabled
        if (vm->trace_execution) {
            printf("          ");
            for (int i = 0; i < vm->stack_top; i++) {
                printf("[ ");
                print_value(vm->stack[i]);
                printf(" ]");
            }
            printf("\n");
            disassemble_instruction(bc, vm->ip);
        }
        
        Instruction instr = bc->instructions[vm->ip++];
        
        switch (instr.opcode) {
            case OP_CONSTANT:
                push(vm, bc->constants[instr.operand]);
                break;
                
            case OP_DISPLAY:
                print_value(pop(vm));
                printf("\n");
                break;
                
            case OP_ADD: {
                Value b = pop(vm);
                Value a = pop(vm);
                push(vm, NUMBER_VAL(AS_NUMBER(a) + AS_NUMBER(b)));
                break;
            }
                
            case OP_HALT:
                return;
                
            default:
                fprintf(stderr, "Unknown opcode: %d\n", instr.opcode);
                exit(1);
        }
    }
}
