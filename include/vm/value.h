#ifndef VALUE_H
#define VALUE_H

#include <stdbool.h>

// TODO: Optimize later with tagged pointers for better performance

typedef enum {
    VAL_NUMBER,
    VAL_STRING,
    VAL_BOOL,
    VAL_NIL,
} ValueType;

typedef struct {
    ValueType type;
    union {
        double number;
        char* string;
        bool boolean;
    } as;
} Value;

// Type checking macros
#define IS_NUMBER(value)  ((value).type == VAL_NUMBER)
#define IS_STRING(value)  ((value).type == VAL_STRING)
#define IS_BOOL(value)    ((value).type == VAL_BOOL)
#define IS_NIL(value)     ((value).type == VAL_NIL)

// Value extraction macros
#define AS_NUMBER(value)  ((value).as.number)
#define AS_STRING(value)  ((value).as.string)
#define AS_BOOL(value)    ((value).as.boolean)

// Value construction macros
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})
#define STRING_VAL(value) ((Value){VAL_STRING, {.string = value}})
#define BOOL_VAL(value)   ((Value){VAL_BOOL, {.boolean = value}})
#define NIL_VAL           ((Value){VAL_NIL, {.number = 0}})

void print_value(Value value);

#endif // VALUE_H
