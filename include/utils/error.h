#ifndef ERROR_H
#define ERROR_H

#include <stdbool.h>

void init_error(const char *filename);
void report_error(int line, int column, const char *message, ...);
void error(int line, const char *message, ...);
bool had_error(void);
void cleanup_error(void);

#endif // ERROR_H
