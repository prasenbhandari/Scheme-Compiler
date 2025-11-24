#ifndef HASH_H
#define HASH_H

#include <stdint.h>

// FNV-1a hash function
uint32_t hash_string(const char* key, int length);

#endif // HASH_H
