#ifndef ERROR_H
#define ERROR_H

typedef struct {
    int line;
    int column;
    const char* message;
} scanner_error;

void init_error(void);
void report_error(const char* message);
int get_current_line(void);
int get_current_column(void);
void set_position(int line, int column);
void cleanup_error(void);

#endif // ERROR_H
