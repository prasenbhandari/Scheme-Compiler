#ifndef BUFFER_H
#define BUFFER_H

#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>

#define BUFFER_SIZE 4096

void init_buffer(FILE* file);
char get_next_char(void);
void unget_char(void);
char peek_char(void);
char skip_whitespace(void);
void cleanup_buffer(void);

// Position tracking
uint32_t get_current_line(void);
uint32_t get_current_column(void);

// Buffer internals (for scanner)
char* get_current_buffer(void);
int32_t* get_buffer_pos(void);

#endif // BUFFER_H
