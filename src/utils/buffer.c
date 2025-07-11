#include <stdio.h>
#include "buffer.h"
#include "error.h"

static char buffers[2][BUFFER_SIZE];
static int current_buffer = 0;
static int buffer_pointer = 0;
static int buffer_end[2] = {0, 0};
static int eof_reached = 0;

void init_buffer(void) {
    current_buffer = 0;
    buffer_pointer = 0;
    buffer_end[0] = buffer_end[1] = 0;
    eof_reached = 0;
}

int fill_buffer(FILE* file, int buffer_num) {
    size_t bytes_read = fread(buffers[buffer_num], 1, BUFFER_SIZE - 1, file);
    if (bytes_read < BUFFER_SIZE - 1) {
        if (ferror(file)) {
            report_error("Error reading from file");
            return -1;
        }
        eof_reached = 1;
    }
    buffers[buffer_num][bytes_read] = '\0';
    buffer_end[buffer_num] = bytes_read;
    return bytes_read;
}

int get_next_char(FILE* file) {
    if (buffer_pointer >= buffer_end[current_buffer]) {
        if (eof_reached) {
            return EOF;
        }
        current_buffer = 1 - current_buffer;
        buffer_pointer = 0;
        if (fill_buffer(file, current_buffer) <= 0) {
            return EOF;
        }
    }
    
    int c = buffers[current_buffer][buffer_pointer++];
    if (c == '\n') {
        set_position(get_current_line() + 1, 0);
    } else {
        set_position(get_current_line(), get_current_column() + 1);
    }
    return c;
}

void unget_char(void) {
    if (buffer_pointer > 0) {
        buffer_pointer--;
        if (buffers[current_buffer][buffer_pointer] == '\n') {
            int temp_pointer = buffer_pointer - 1;
            int column = 0;
            while (temp_pointer >= 0 && buffers[current_buffer][temp_pointer] != '\n') {
                column++;
                temp_pointer--;
            }
            set_position(get_current_line() - 1, column);
        } else {
            set_position(get_current_line(), get_current_column() - 1);
        }
    } else if (current_buffer == 1) {
        current_buffer = 0;
        buffer_pointer = buffer_end[current_buffer] - 1;
        // Handle line/column counting similarly
    }
}

char* get_current_buffer(void) {
    return buffers[current_buffer];
}

int* get_buffer_pos(void) {
    return &buffer_pointer;
}

void set_buffer_pos(int pos){
    buffer_pointer = pos;
}

void cleanup_buffer(void) {
    // Reset all buffer state
    init_buffer();
}

buffer_state get_buffer_state(void) {
    buffer_state state;
    state.current_buffer = current_buffer;
    state.buffer_pointer = buffer_pointer;
    state.buffer_end[0] = buffer_end[0];
    state.buffer_end[1] = buffer_end[1];
    state.eof_reached = eof_reached;
    return state;
}

void restore_buffer_state(buffer_state state) {
    current_buffer = state.current_buffer;
    buffer_pointer = state.buffer_pointer;
    buffer_end[0] = state.buffer_end[0];
    buffer_end[1] = state.buffer_end[1];
    eof_reached = state.eof_reached;
}

