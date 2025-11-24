#ifndef TABLE_H
#define TABLE_H

#include "value.h"
#include <stdbool.h>

typedef struct {
    char* key;
    Value value;
} TableEntry;

typedef struct {
    int count;
    int capacity;
    TableEntry* entries;
} Table;

void init_table(Table* table);
void free_table(Table* table);
bool table_set(Table* table, const char* key, Value value);
bool table_get(Table* table, const char* key, Value* value);

#endif // TABLE_H
