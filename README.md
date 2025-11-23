# Scheme-Compiler
A basic stack based scheme compiler written in C

This my learning project for building a scheme compiler. It compiles source code into custom bytecode, which is executed by a custom Virtual Machine.

## Architecture

It consists of five modules mirroring the standard phases of a compiler design:
* **Scanner:** Tokenizes source code into numbers, strings, identifiers, and keywords.
* **Parser:** A recursive descent parser that validates grammar and constructs an Abstract Syntax Tree (AST).
* **Analyzer:** Performs semantic analysis, symbol table management, and scope verification.
* **Code Generator:** Traverses the AST and emits bytecode instructions.
* **Virtual Machine:** A Stack-Based VM that executes the bytecode.
    
## Getting Started

### Prerequisites
* GCC or Clang compiler
* Cmake
    
### Build
```
    cd Scheme-Compiler
    mkdir build
    cmake ..
    make
```

### Run 
```
    ./scheme_compiler <filename.scm>
```

