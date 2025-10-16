#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdbool.h>
#include "../scanner/token.h"

typedef enum {
    SYMBOL_DECLARED,
    SYMBOL_DEFINED,
} symbol_state;


typedef struct symbol {
    token* token;
    symbol_state state;
    struct symbol* next;
} symbol;


typedef struct scope {
    symbol** table;
    int count;
    int capacity;
    struct scope* parent;
} scope;


scope* init_scope(scope* parent);

void free_scope(scope* s);

bool add_symbol(scope* s, token* t);

symbol* find_symbol(scope* s, token* t);

#endif