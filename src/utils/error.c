#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "utils/error.h"
#include "utils/buffer.h" // For position tracking

#define MAX_LINES 10000
#define MAX_LINE_LENGTH 256

static const char *filename;
static char *source_lines[MAX_LINES];
static int line_count = 0;
static bool error_flag = false;

void init_error(const char *fname) {
    filename = fname;
    error_flag = false;
    line_count = 0;

    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open file '%s'.\n", filename);
        exit(1);
    }

    char buffer[MAX_LINE_LENGTH];
    while (line_count < MAX_LINES && fgets(buffer, sizeof(buffer), file)) {
        source_lines[line_count] = strdup(buffer);
        if (!source_lines[line_count]) {
            fprintf(stderr, "Error: Memory allocation failed.\n");
            exit(1);
        }
        line_count++;
    }

    fclose(file);
}

static void vreport(int line, int column, const char *fmt, va_list args) {
    error_flag = true;
    fprintf(stderr, "%s:%d:%d: error: ", filename, line, column);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");

    if (line > 0 && line <= line_count) {
        fprintf(stderr, "%4d | %s", line, source_lines[line - 1]);
        if (column > 0) {
            fprintf(stderr, "%*s^", column + 6, "");
        }
        fprintf(stderr, "\n");
    }
}

void report_error(int line, int column, const char *message, ...) {
    va_list args;
    va_start(args, message);
    vreport(line, column, message, args);
    va_end(args);
}

void error(int line, const char *message, ...) {
    va_list args;
    va_start(args, message);
    vreport(line, 0, message, args);
    va_end(args);
}

bool had_error(void) {
    return error_flag;
}

void cleanup_error(void) {
    for (int i = 0; i < line_count; i++) {
        free(source_lines[i]);
        source_lines[i] = NULL;
    }
    line_count = 0;
    error_flag = false;
    filename = NULL;
}
