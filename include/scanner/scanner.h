#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>
#include "token.h"

// Scanner functions
void init_scanner(void);
void cleanup_scanner(void);
token* next_token(FILE* file);
int fill_buffer(FILE* file, int buffer_num);
char* get_current_buffer(void);
int* get_buffer_pos(void);

#endif // SCANNER_H
