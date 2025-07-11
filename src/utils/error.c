#include <stdio.h>
#include "error.h"

static scanner_error current_error = {0, 0, NULL};
static int current_line = 1;
static int current_column = 0;

void init_error(void) {
    current_line = 1;
    current_column = 0;
    current_error.line = 0;
    current_error.column = 0;
    current_error.message = NULL;
}

void report_error(const char* message) {
    current_error.line = current_line;
    current_error.column = current_column;
    current_error.message = message;
    fprintf(stderr, "Error at line %d, column %d: %s\n", 
            current_line, current_column, message);
}

int get_current_line(void) {
    return current_line;
}

int get_current_column(void) {
    return current_column;
}

void set_position(int line, int column) {
    current_line = line;
    current_column = column;
}

void cleanup_error(void) {
    init_error();
}
