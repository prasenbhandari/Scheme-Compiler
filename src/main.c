#include <stdio.h>
#include "scanner.h"
#include "buffer.h"

//int argc, char *argv[]

int main() {
    /* if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    } */

    FILE* file = fopen("/home/prasen/Programing/C/Project/Compiler/temp.scm", "r");
    if(!file) {
        fprintf(stderr, "Failed to open file: %s\n", "temp.scm");
        return 1;
    }
    
    init_scanner(); // Use scanner init instead of separate buffer/error inits

    if (fill_buffer(file, 0) <= 0) {
        fclose(file);
        return 1;
    }

    token* t;
    while ((t = next_token(file)) != NULL) {
        if (t->type == TOKEN_DEC) {
            printf("Token: %d\tType: %s\n", t->int_value, token_type_to_string(t->type));
        } else if (t->type == TOKEN_REAL) {
            printf("Token: %f\tType: %s\n", t->real_value, token_type_to_string(t->type));
        } else {
            printf("Token: %s\tType: %s\n", t->lexeme, token_type_to_string(t->type));
        }
        free_token(t);
    }

    cleanup_scanner(); // Use scanner cleanup
    fclose(file);
    return 0;
}
