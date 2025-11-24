#include <stdlib.h>
#include "vm/table.h"
#include "utils/memory.h"
#include "utils/hash.h"
#include "stdint.h"
#include "string.h"

void init_table(Table* table) {
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}


void free_table(Table* table) {
    for (int i = 0; i < table->capacity; i++) {
        free(table->entries[i].key);
    }
    free(table->entries);
}


static TableEntry* find_entry(TableEntry* entries, int capacity, const char* key){
    uint32_t hash = hash_string(key, strlen(key));
    uint32_t index = hash % capacity;

    for (;;) {
        TableEntry* entry = &entries[index];
        if (entry->key == NULL) return entry;

        if (strcmp(entry->key, key) == 0) return entry;

        index = (index + 1) % capacity;
    }
}


static void adjust_capacity(Table* table, int capacity){

    TableEntry* entries = (TableEntry*)calloc(capacity, sizeof(TableEntry));

    int old_capacity = table->capacity;
    table->count = 0;

    for(int i = 0; i < old_capacity; i++){
        TableEntry* entry = &table->entries[i];
        if (entry->key == NULL) continue;

        TableEntry* dest = find_entry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }

    FREE_ARRAY(TableEntry, table->entries, old_capacity);
    table->entries = entries;
    table->capacity = capacity;
    
}

bool table_set(Table* table, const char* key, Value value) {
    
    if(table->count + 1 > table->capacity * 0.75){
        int old_capacity = table->capacity;
        int new_capacity = GROW_CAPACITY(old_capacity);
        adjust_capacity(table, new_capacity);
    }

    TableEntry* entry = find_entry(table->entries, table->capacity, key);
    bool is_new_key = entry->key == NULL;

    if(is_new_key){
        table->count++;
        entry->key = strdup(key);
    }
    
    entry->value = value;
    return is_new_key;

}

bool table_get(Table* table, const char* key, Value* value){
    if (table->count == 0) return false;

    TableEntry* entry = find_entry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;

    *value = entry->value;
    return true;
}


