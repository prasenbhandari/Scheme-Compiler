#include <stdio.h>
#include <stdlib.h>
#include "scanner/scanner.h"
#include "scanner/token.h"
#include "utils/buffer.h"
#include "utils/error.h"
#include "parser/parser.h"
#include "analyzer/analyzer.h"

// Function to print the AST tree
void print_ast(ast_node* node, int depth) {
    if (!node) {
        for (int i = 0; i < depth; i++) printf("  ");
        printf("NULL (ERROR)\n");
        return;
    }
    
    for (int i = 0; i < depth; i++) printf("  ");
    
    if (node->type == NODE_ATOM) {
        printf("ATOM: ");
        if (node->token->type == TOKEN_DEC) {
            printf("%d\n", node->token->int_value);
        } else if (node->token->type == TOKEN_REAL) {
            printf("%f\n", node->token->real_value);
        } else {
            printf("%s\n", node->token->lexeme);
        }
    } else if (node->type == NODE_LIST) {
        printf("LIST:\n");
        for (int i = 0; i < depth; i++) printf("  ");
        printf("  car:\n");
        print_ast(node->car, depth + 2);
        for (int i = 0; i < depth; i++) printf("  ");
        printf("  cdr:\n");
        print_ast(node->cdr, depth + 2);
    } else if (node->type == NODE_NIL) {
        printf("NIL\n");
    } else {
        printf("UNKNOWN NODE TYPE\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    init_error(argv[1]);
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        report_error(0, 0, "Could not open file '%s'", argv[1]);
        return 1;
    }

    parser* p = init_parser(file);
    if (!p) {
        fprintf(stderr, "Failed to initialize parser\n");
        fclose(file);
        return 1;
    }

    analyzer* a = init_analyzer();
    if (!a){
        fprintf(stderr, "Failed to initialize analyzer\n");
        fclose(file);
        return 1;
    }

    // Check for errors during scanning initialization
    if (had_error()) {
        fprintf(stderr, "\nCompilation failed with errors.\n");
        cleanup_scanner();
        fclose(file);
        return 1;
    }

    // Parse and print AST
    printf("Parsing file: %s\n", argv[1]);
    printf("=================\n\n");
    
    int expression_count = 0;
    
    while (p->current != NULL) {
        ast_node* ast = parse_expression(p);
        
        if (!ast) {
            if (!p->current) {
                break;
            }
            continue;
        }

        // Analyze the AST
        if (!analyze_ast(a, ast)) {
            fprintf(stderr, "Failed to analyze AST\n");
            continue;
        }

        if (ast) {
            p->panic_mode = false;
            printf("Expression %d:\n", ++expression_count);
            print_ast(ast, 0);
            printf("\n");
            
            // Free the AST after we're done with it
            free_ast(ast);
        } else {
            printf("Failed to parse expression\n");
            break;
        }
    }

    cleanup_scanner();
    free_parser(p);
    free_analyzer(a);
    fclose(file);

    return had_error() ? 1 : 0;
}
