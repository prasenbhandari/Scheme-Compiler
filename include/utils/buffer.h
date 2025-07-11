#ifndef BUFFER_H
#define BUFFER_H

#include <stdio.h>

#define BUFFER_SIZE 4096

typedef struct buffer_state{
    int current_buffer;
    int buffer_pointer;
    int buffer_end[2];
    int eof_reached;
} buffer_state;

buffer_state get_buffer_state(void);
void restore_buffer_state(buffer_state state);
void init_buffer(void);
int fill_buffer(FILE* file, int buffer_num);
int get_next_char(FILE* file);
void unget_char(void);
void cleanup_buffer(void);
char* get_current_buffer(void);
int* get_buffer_pos(void);
void set_buffer_pos(int pos);

#endif // BUFFER_H
