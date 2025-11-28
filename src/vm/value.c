#include "vm/value.h"
#include <stdio.h>

void print_value(Value value) {
    switch (value.type) {
        case VAL_NUMBER:
            printf("%g", AS_NUMBER(value));
            break;
        case VAL_STRING:
            printf("%s", AS_STRING(value));
            break;
        case VAL_BOOL:
            printf(AS_BOOL(value) ? "#t" : "#f");
            break;
        case VAL_PAIR:{
            printf("(");
            print_value(AS_PAIR(value)->car);

            Value current = AS_PAIR(value)->cdr;
            while(IS_PAIR(current)){
                printf(" ");
                print_value(AS_PAIR(current)->car);
                current = AS_PAIR(current)->cdr;
            }

            if(!IS_NIL(current)){
                printf(" . ");
                print_value(current);
            }
            
            printf(")");
            break;
        }
            
        case VAL_NIL:
            printf("()");
            break;
        case VAL_FUNCTION:
            printf("<fn %s>", AS_FUNCTION(value)->name ? AS_FUNCTION(value)->name : "lambda");
            break;
        case VAL_CLOSURE:
            printf("<fn %s>", AS_CLOSURE(value)->function->name ? AS_CLOSURE(value)->function->name : "lambda");
            break;
    }
}

