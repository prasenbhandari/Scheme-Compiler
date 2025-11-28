#include <stdio.h>
#include <ctype.h>
#include "utils/buffer.h"
#include "utils/error.h"

static char buffers[2][BUFFER_SIZE];
static int current_buffer_idx = 0;
static int32_t buffer_pos = 0;
static int buffer_limits[2] = {0, 0};
static int eof_reached = 0;
static FILE *file_handle = NULL;

// Position tracking
static uint32_t current_line = 1;
static uint32_t current_column = 0;

// Forward declaration
static int fill_buffer(int buffer_num);

void init_buffer(FILE* file) {
    file_handle = file;
    current_buffer_idx = 0;
    buffer_pos = 0;
    buffer_limits[0] = buffer_limits[1] = 0;
    eof_reached = 0;
    current_line = 1;
    current_column = 0;
    fill_buffer(0);
}

int fill_buffer(int buffer_num) {
    if (!file_handle) return -1;
    size_t bytes_read = fread(buffers[buffer_num], 1, BUFFER_SIZE - 1, file_handle);
    if (bytes_read < BUFFER_SIZE - 1) {
        if (ferror(file_handle)) {
            report_error(current_line, current_column, "Error reading from file");
            return -1;
        }
        eof_reached = 1;
    }
    buffers[buffer_num][bytes_read] = '\0';
    buffer_limits[buffer_num] = bytes_read;
    return bytes_read;
}

char get_next_char(void) {
    if (buffer_pos >= buffer_limits[current_buffer_idx]) {
        if (eof_reached) {
            return EOF;
        }
        current_buffer_idx = 1 - current_buffer_idx;
        buffer_pos = 0;
        if (fill_buffer(current_buffer_idx) <= 0) {
            return EOF;
        }
    }
    
    char c = buffers[current_buffer_idx][buffer_pos++];
    if (c == '\n') {
        current_line++;
        current_column = 0;
    } else {
        current_column++;
    }
    return c;
}

void unget_char(void) {
    if (buffer_pos > 0) {
        buffer_pos--;
        char c = buffers[current_buffer_idx][buffer_pos];
        if (c == '\n') {
            current_line--;
            // Column restoration is complex, so just set to 0 for simplicity
            current_column = 0;
        } else if (current_column > 0) {
            current_column--;
        }
    }
}

char peek_char(void) {
    if (buffer_pos >= buffer_limits[current_buffer_idx]) {
        if (eof_reached) return EOF;
        return EOF; 
    }
    return buffers[current_buffer_idx][buffer_pos];
}

char skip_whitespace(void) {
    char c;
    while (isspace(c = peek_char()) && c != EOF) {
        get_next_char(); // Consume the whitespace
    }
    // Return the first non-whitespace character without consuming it
    return c;
}

uint32_t get_current_line(void) {
    return current_line;
}

uint32_t get_current_column(void) {
    return current_column;
}

char* get_current_buffer(void) {
    return buffers[current_buffer_idx];
}

int32_t* get_buffer_pos(void) {
    return &buffer_pos;
}

void cleanup_buffer(void) {
    file_handle = NULL;
    current_buffer_idx = 0;
    buffer_pos = 0;
    buffer_limits[0] = buffer_limits[1] = 0;
    eof_reached = 0;
    current_line = 1;
    current_column = 0;
}

