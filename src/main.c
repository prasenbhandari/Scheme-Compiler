#include <stdio.h>
#include <stdlib.h>
#include "scanner/scanner.h"
#include "scanner/token.h"
#include "utils/buffer.h"
#include "utils/error.h"
#include "parser/parser.h"
#include "analyzer/analyzer.h"
#include "vm/vm.h"
#include "vm/instruction.h"
#include "vm/value.h"

// Function to print the AST tree
void print_ast(AstNode* node, int depth) {
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

// Test function for VM - manually create bytecode
void test_vm() {
    printf("=== VM Test ===\n");
    
    // Test 1: (display "hello")
    printf("Test 1: (display \"hello\")\n");
    Bytecode bc1;
    init_bytecode(&bc1);
    int idx1 = add_constant(&bc1, STRING_VAL("hello"));
    emit_instruction(&bc1, OP_CONSTANT, idx1);
    emit_instruction(&bc1, OP_DISPLAY, 0);
    emit_instruction(&bc1, OP_HALT, 0);
    
    VM vm1;
    init_vm(&vm1);
    vm_execute(&vm1, &bc1);
    free_vm(&vm1);
    free_bytecode(&bc1);
    
    // Test 2: (display 42)
    printf("\nTest 2: (display 42)\n");
    Bytecode bc2;
    init_bytecode(&bc2);
    int idx2 = add_constant(&bc2, NUMBER_VAL(42));
    emit_instruction(&bc2, OP_CONSTANT, idx2);
    emit_instruction(&bc2, OP_DISPLAY, 0);
    emit_instruction(&bc2, OP_HALT, 0);
    
    VM vm2;
    init_vm(&vm2);
    vm_execute(&vm2, &bc2);
    free_vm(&vm2);
    free_bytecode(&bc2);
    
    // Test 3: (display (+ 1 2))
    printf("\nTest 3: (display (+ 1 2))\n");
    Bytecode bc3;
    init_bytecode(&bc3);
    int idx3_a = add_constant(&bc3, NUMBER_VAL(1));
    int idx3_b = add_constant(&bc3, NUMBER_VAL(2));
    emit_instruction(&bc3, OP_CONSTANT, idx3_a);
    emit_instruction(&bc3, OP_CONSTANT, idx3_b);
    emit_instruction(&bc3, OP_ADD, 0);
    emit_instruction(&bc3, OP_DISPLAY, 0);
    emit_instruction(&bc3, OP_HALT, 0);
    
    VM vm3;
    init_vm(&vm3);
    vm_execute(&vm3, &bc3);
    free_vm(&vm3);
    free_bytecode(&bc3);
    
    printf("\n=== VM Test Complete ===\n\n");
}

int main(int argc, char *argv[]) {
    // Run VM test first
    test_vm();
    
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

    Parser* p = init_parser(file);
    if (!p) {
        fprintf(stderr, "Failed to initialize parser\n");
        fclose(file);
        return 1;
    }

    Analyzer* a = init_analyzer();
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
        AstNode* ast = parse_expression(p);
        
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
