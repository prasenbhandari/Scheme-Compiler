#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdbool.h>
#include "../scanner/token.h"

typedef enum {
    SYMBOL_DECLARED,
    SYMBOL_DEFINED,
} SymbolState;


typedef struct Symbol {
    Token* token;
    SymbolState state;
    struct Symbol* next;
} Symbol;


typedef struct Scope {
    Symbol** table;
    int count;
    int capacity;
    struct Scope* parent;
} Scope;


Scope* init_scope(Scope* parent);

void free_scope(Scope* s);

bool add_symbol(Scope* s, Token* t);

Symbol* find_symbol(Scope* s, Token* t);

#endif