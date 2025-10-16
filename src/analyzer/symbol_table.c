#include <string.h>
#include <stdlib.h>
#include "../../include/analyzer/symbol_table.h"

#define INITIAL_CAPACITY 8

static unsigned int hash_string(const char* key, int length){
    unsigned int hash = 2166136261u;

    for (int i = 0; i < length; i++ ){
        hash ^= (unsigned char)key[i];
        hash *= 16777619;
    }

    return hash;
}


scope* init_scope(scope* parent) {
    scope* s = (scope*)malloc(sizeof(scope));
    s->parent = parent;
    s->count = 0;
    s->capacity = INITIAL_CAPACITY;
    s->table = (symbol**)calloc(s->capacity, sizeof(symbol*));
    return s;
}


void free_scope(scope* s){
    for (int i = 0; i < s->capacity; i++){
        symbol* sym = s->table[i];
        while(sym){
            symbol* next = sym->next;
            free(sym);
            sym = next;
        }
    }

    free(s->table);
    free(s);
}


bool add_symbol(scope* s, token* t){
    unsigned int hash = hash_string(t->lexeme, strlen(t->lexeme));
    unsigned int index = hash % s->capacity;

    symbol* new_symbol = (symbol*)malloc(sizeof(symbol));
    new_symbol->token = t;
    new_symbol->state = SYMBOL_DECLARED;
    new_symbol->next = s->table[index];
    s->table[index] = new_symbol;
    s->count++;

    return true;
}


static symbol* find_in_scope(scope* s, token* t){
    unsigned int hash = hash_string(t->lexeme, strlen(t->lexeme));
    unsigned int index = hash % s->capacity;

    symbol* sym = s->table[index];
    while(sym){
        if(strcmp(t->lexeme, sym->token->lexeme) == 0){
            return sym;
        }

        sym = sym->next;
    }

    return NULL;
}


symbol* find_symbol(scope* s, token* t){
    scope* current = s;
    while(current){
        symbol* sym = find_in_scope(current, t);
        if (sym){
            return sym;
        }
        current = current->parent;
    }

    return NULL;
}