#include <string.h>
#include <stdlib.h>
#include "../../include/analyzer/symbol_table.h"
#include "utils/hash.h"

#define INITIAL_CAPACITY 8


Scope* init_scope(Scope* parent) {
    Scope* s = (Scope*)malloc(sizeof(Scope));
    s->parent = parent;
    s->count = 0;
    s->capacity = INITIAL_CAPACITY;
    s->table = (Symbol**)calloc(s->capacity, sizeof(Symbol*));
    return s;
}


void free_scope(Scope* s){
    for (int i = 0; i < s->capacity; i++){
        Symbol* sym = s->table[i];
        while(sym){
            Symbol* next = sym->next;
            free(sym);
            sym = next;
        }
    }

    free(s->table);
    free(s);
}


bool add_symbol(Scope* s, Token* t){
    unsigned int hash = hash_string(t->lexeme, strlen(t->lexeme));
    unsigned int index = hash % s->capacity;

    Symbol* new_symbol = (Symbol*)malloc(sizeof(Symbol));
    new_symbol->token = t;
    new_symbol->state = SYMBOL_DECLARED;
    new_symbol->next = s->table[index];
    s->table[index] = new_symbol;
    s->count++;

    return true;
}


static Symbol* find_in_scope(Scope* s, Token* t){
    unsigned int hash = hash_string(t->lexeme, strlen(t->lexeme));
    unsigned int index = hash % s->capacity;

    Symbol* sym = s->table[index];
    while(sym){
        if(strcmp(t->lexeme, sym->token->lexeme) == 0){
            return sym;
        }

        sym = sym->next;
    }

    return NULL;
}


Symbol* find_symbol(Scope* s, Token* t){
    Scope* current = s;
    while(current){
        Symbol* sym = find_in_scope(current, t);
        if (sym){
            return sym;
        }
        current = current->parent;
    }

    return NULL;
}