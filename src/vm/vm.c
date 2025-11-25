#include "vm/vm.h"
#include "vm/debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

// Runtime error reporting
static void runtime_error(VM* vm, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    fprintf(stderr, "Runtime error at instruction %d: ", vm->ip);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    
    va_end(args);
}

void init_vm(VM* vm) {
    vm->stack_top = 0;
    vm->ip = 0;
    vm->code = NULL;
    vm->trace_execution = false;  // Tracing disabled by default
    init_table(&vm->globals);
}

void free_vm(VM* vm) {
    free_table(&vm->globals);
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

// Type-safe pop functions with runtime checking
static Value pop_number(VM* vm) {
    Value v = pop(vm);
    if (!IS_NUMBER(v)) {
        runtime_error(vm, "Type error: Expected number, got %s", 
                     IS_STRING(v) ? "string" : 
                     IS_BOOL(v) ? "boolean" : "other");
        exit(1);
    }
    return v;
}

static Value pop_any(VM* vm) {
    return pop(vm);
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

            case OP_CONS: {
                Value cdr = pop(vm);
                Value car = pop(vm);
                
                ObjPair* pair = malloc(sizeof(ObjPair));
                pair->car = car;
                pair->cdr = cdr;
                push(vm, PAIR_VAL(pair));
                break;
            }

            case OP_CAR: {
                Value pair = pop(vm);
                if (!IS_PAIR(pair)) {
                    runtime_error(vm, "Type error: Expected pair, got %s", 
                                 IS_STRING(pair) ? "string" : 
                                 IS_BOOL(pair) ? "boolean" : "other");
                    exit(1);
                }
                push(vm, AS_PAIR(pair)->car);
                break;
            }

            case OP_CDR: {
                Value pair = pop(vm);
                if (!IS_PAIR(pair)) {
                    runtime_error(vm, "Type error: Expected pair, got %s", 
                                 IS_STRING(pair) ? "string" : 
                                 IS_BOOL(pair) ? "boolean" : "other");
                    exit(1);
                }
                push(vm, AS_PAIR(pair)->cdr);
                break;
            }
                
            case OP_READ: {
                double num;
                if (scanf("%lf", &num) == 1) {
                    push(vm, NUMBER_VAL(num));
                } else {
                    runtime_error(vm, "Failed to read number from input");
                    exit(1);
                }
                break;
            }
                
            case OP_READ_LINE: {
                char buffer[1024];
                if (fgets(buffer, sizeof(buffer), stdin)) {
                    // Remove trailing newline if present
                    size_t len = strlen(buffer);
                    if (len > 0 && buffer[len - 1] == '\n') {
                        buffer[len - 1] = '\0';
                    }
                    // Allocate and store string
                    char* str = strdup(buffer);
                    if (!str) {
                        runtime_error(vm, "Memory allocation failed");
                        exit(1);
                    }
                    push(vm, STRING_VAL(str));
                } else {
                    runtime_error(vm, "Failed to read line from input");
                    exit(1);
                }
                break;
            }
                
            case OP_ADD: {
                Value b = pop_number(vm);
                Value a = pop_number(vm);
                push(vm, NUMBER_VAL(AS_NUMBER(a) + AS_NUMBER(b)));
                break;
            }

            case OP_SUB: {
                Value b = pop_number(vm);
                Value a = pop_number(vm);
                push(vm, NUMBER_VAL(AS_NUMBER(a) - AS_NUMBER(b)));
                break;
            }

            case OP_MUL: {
                Value b = pop_number(vm);
                Value a = pop_number(vm);
                push(vm, NUMBER_VAL(AS_NUMBER(a) * AS_NUMBER(b)));
                break;
            }

            case OP_DIV: {
                Value b = pop_number(vm);
                Value a = pop_number(vm);
                if (AS_NUMBER(b) == 0) {
                    runtime_error(vm, "Division by zero");
                    exit(1);
                }
                push(vm, NUMBER_VAL(AS_NUMBER(a) / AS_NUMBER(b)));
                break;
            }

            case OP_LESS: {
                Value b = pop_number(vm);
                Value a = pop_number(vm);
                push(vm, BOOL_VAL(AS_NUMBER(a) < AS_NUMBER(b)));
                break;
            }

            case OP_GREATER: {
                Value b = pop_number(vm);
                Value a = pop_number(vm);
                push(vm, BOOL_VAL(AS_NUMBER(a) > AS_NUMBER(b)));
                break;
            }

            case OP_EQUAL: {
                Value b = pop_number(vm);
                Value a = pop_number(vm);
                push(vm, BOOL_VAL(AS_NUMBER(a) == AS_NUMBER(b)));
                break;
            }

            case OP_LESS_EQUAL: {
                Value b = pop_number(vm);
                Value a = pop_number(vm);
                push(vm, BOOL_VAL(AS_NUMBER(a) <= AS_NUMBER(b)));
                break;
            }

            case OP_GREATER_EQUAL: {
                Value b = pop_number(vm);
                Value a = pop_number(vm);
                push(vm, BOOL_VAL(AS_NUMBER(a) >= AS_NUMBER(b)));
                break;
            }

            case OP_NOT_EQUAL: {
                Value b = pop_number(vm);
                Value a = pop_number(vm);
                push(vm, BOOL_VAL(AS_NUMBER(a) != AS_NUMBER(b)));
                break;
            }

            case OP_JUMP_IF_FALSE: {
                Value condition = pop_any(vm);
                if (IS_BOOL(condition) && !AS_BOOL(condition)) {
                    vm->ip = instr.operand;
                }
                break;
            }

            case OP_JUMP:
                vm->ip = instr.operand;
                break;

            case OP_HALT:
                return;

            case OP_POP:
                pop(vm);
                break;

            case OP_JUMP_IF_TRUE_OR_POP: {
                Value v = peek_stack(vm, 0);
                if (!(IS_BOOL(v) && !AS_BOOL(v))) {
                    vm->ip = instr.operand;
                } else {
                    pop(vm);
                }
                break;
            }

            case OP_JUMP_IF_FALSE_OR_POP: {
                Value v = peek_stack(vm, 0);
                if (IS_BOOL(v) && !AS_BOOL(v)) {
                    vm->ip = instr.operand;
                } else {
                    pop(vm);
                }
                break;
            }

            case OP_DEFINE_GLOBAL: {
                Value name_val = bc->constants[instr.operand];
                
                if (!IS_STRING(name_val)) {
                    runtime_error(vm, "Fatal: Variable name is not a string");
                    exit(1);
                }
                
                char* name = AS_STRING(name_val);
                Value value = pop_any(vm);
                table_set(&vm->globals, name, value);
                break;
            }

            case OP_GET_GLOBAL: {
                Value name_val = bc->constants[instr.operand];
                
                if (!IS_STRING(name_val)) {
                    runtime_error(vm, "Fatal: Variable name is not a string");
                    exit(1);
                }
                
                char* name = AS_STRING(name_val);
                Value value;
                if (!table_get(&vm->globals, name, &value)) {
                    runtime_error(vm, "Undefined variable '%s'", name);
                    exit(1);
                }
                push(vm, value);
                break;
            }

            case OP_SET_GLOBAL: {
                Value name_val = bc->constants[instr.operand];
                
                if (!IS_STRING(name_val)) {
                    runtime_error(vm, "Fatal: Variable name is not a string");
                    exit(1);
                }
                
                char* name = AS_STRING(name_val);
                Value value = pop_any(vm);
                
                Value dummy;
                if (!table_get(&vm->globals, name, &dummy)) {
                    runtime_error(vm, "Undefined variable '%s'", name);
                    exit(1);
                }
                
                table_set(&vm->globals, name, value);
                break;
            }
                
            default:
                fprintf(stderr, "Unknown opcode: %d\n", instr.opcode);
                exit(1);
        }
    }
}
