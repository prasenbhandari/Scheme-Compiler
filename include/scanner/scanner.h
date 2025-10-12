#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>
#include "token.h"

// Scanner functions
void init_scanner(FILE *file);
void cleanup_scanner(void);
token* next_token(void);

#endif // SCANNER_H
