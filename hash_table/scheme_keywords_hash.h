
#ifndef SCHEME_KEYWORDS_HASH_H
#define SCHEME_KEYWORDS_HASH_H

#include <stddef.h>  // Required for size_t

struct Keyword {
    const char *name;
    int token;
};

/* Proper function declaration */
const struct Keyword *lookup_keyword(const char *str, size_t len);

#endif /* SCHEME_KEYWORDS_HASH_H */
