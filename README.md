# Scheme-Compiler

A basic stack based scheme compiler written in C

This my learning project for building a scheme compiler. It compiles source code into custom bytecode, which is executed by a custom Virtual Machine.

## Architecture

It consists of five modules mirroring the standard phases of a compiler design:

- **Scanner:** Tokenizes source code into numbers, strings, identifiers, and keywords.
- **Parser:** A recursive descent parser that validates grammar and constructs an Abstract Syntax Tree (AST).
- **Analyzer:** Performs semantic analysis, symbol table management, and scope verification.
- **Code Generator:** Traverses the AST and emits bytecode instructions.
- **Virtual Machine:** A Stack-Based VM that executes the bytecode.

## Getting Started

### Prerequisites

- GCC or Clang compiler
- Cmake

### Build

```
    cd Scheme-Compiler
    mkdir build && cd build
    cmake ..
    make
```

### Run

```
    ./scheme_compiler <filename.scm>
```

## Implemented Language Features

### Data Types

- **Numbers**: Integers and floating-point numbers
- **Strings**: String literals with double quotes
- **Booleans**: `#t` (true) and `#f` (false)
- **Nil**: Empty value

### Special Forms

- **`define`**: Global variable definitions

  ```scheme
  (define x 10)
  (define name "Alice")
  ```

- **`if`**: Conditional expressions

  ```scheme
  (if (> x 5) "big" "small")
  ```

- **`cond`**: Multi-way conditionals

  ```scheme
  (cond
    ((< x 0) "negative")
    ((= x 0) "zero")
    (else "positive"))
  ```

- **`and`**: Logical AND with short-circuit evaluation

  ```scheme
  (and #t #t (> 5 3))  ; => #t
  ```

- **`or`**: Logical OR with short-circuit evaluation
  ```scheme
  (or #f #f (> 5 3))  ; => #t
  ```

- **`quote`**: Quote special form to prevent evaluation
  ```scheme
  (quote (1 2 3))     ; => (1 2 3)
  '(+ 1 2)            ; => (+ 1 2) [not evaluated]
  '(a b c)            ; => (a b c)
  ```

### Arithmetic Operators (Variadic)

- **`+`**: Addition (0 or more arguments)

  ```scheme
  (+)           ; => 0
  (+ 1 2 3 4)   ; => 10
  ```

- **`-`**: Subtraction (1 or more arguments)

  ```scheme
  (- 10)        ; => -10 (negation)
  (- 10 3 2)    ; => 5
  ```

- **`*`**: Multiplication (0 or more arguments)

  ```scheme
  (*)           ; => 1
  (* 2 3 4)     ; => 24
  ```

- **`/`**: Division (1 or more arguments)
  ```scheme
  (/ 100)       ; => 0.01 (reciprocal)
  (/ 100 2 5)   ; => 10
  ```

### Comparison Operators (Binary)

- `<`, `>`, `=`, `<=`, `>=`, `!=`
  ```scheme
  (< 5 10)      ; => #t
  (= 5 5)       ; => #t
  ```

### List Operations

- **`cons`**: Construct a new list by prepending an element
  ```scheme
  (cons 1 (cons 2 (cons 3 '())))  ; => (1 2 3)
  (cons 'a '(b c))                ; => (a b c)
  ```

- **`car`**: Get the first element (head) of a list
  ```scheme
  (car '(1 2 3))   ; => 1
  (car '(a b c))   ; => a
  ```

- **`cdr`**: Get the rest of the list (tail)
  ```scheme
  (cdr '(1 2 3))   ; => (2 3)
  (cdr '(a b c))   ; => (b c)
  ```

### I/O Functions

- **`display`**: Print a value

  ```scheme
  (display "Hello, World!")
  ```

- **`read`**: Read a number from stdin

  ```scheme
  (define x (read))
  ```

- **`read-line`**: Read a line of text from stdin
  ```scheme
  (define name (read-line))
  ```
