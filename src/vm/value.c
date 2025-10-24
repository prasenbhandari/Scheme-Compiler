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
        case VAL_NIL:
            printf("()");
            break;
    }
}

