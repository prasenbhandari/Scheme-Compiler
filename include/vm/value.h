#ifndef VALUE_H
#define VALUE_H

#include <stdbool.h>

typedef struct ObjPair ObjPair;

typedef enum {
    VAL_NUMBER,
    VAL_STRING,
    VAL_BOOL,
    VAL_NIL,
    VAL_PAIR,      // For cons cells/lists
    VAL_PROCEDURE, // For functions/lambdas
    VAL_ANY,       // For semantic analysis - accepts any type
} ValueType;

typedef struct {
    ValueType type;
    union {
        double number;
        char* string;
        bool boolean;
        ObjPair* pair;
    } as;
} Value;

struct ObjPair {
    Value car;
    Value cdr;
};

// Type checking macros
#define IS_NUMBER(value)  ((value).type == VAL_NUMBER)
#define IS_STRING(value)  ((value).type == VAL_STRING)
#define IS_BOOL(value)    ((value).type == VAL_BOOL)
#define IS_PAIR(value)    ((value).type == VAL_PAIR)
#define IS_NIL(value)     ((value).type == VAL_NIL)

// Value extraction macros
#define AS_NUMBER(value)  ((value).as.number)
#define AS_STRING(value)  ((value).as.string)
#define AS_BOOL(value)    ((value).as.boolean)
#define AS_PAIR(value)    ((value).as.pair)

// Value construction macros
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})
#define STRING_VAL(value) ((Value){VAL_STRING, {.string = (char*)(value)}})
#define BOOL_VAL(value)   ((Value){VAL_BOOL, {.boolean = value}})
#define PAIR_VAL(pair)    ((Value){VAL_PAIR, {.pair = pair}})   
#define NIL_VAL           ((Value){VAL_NIL, {.number = 0}})

void print_value(Value value);

#endif // VALUE_H
