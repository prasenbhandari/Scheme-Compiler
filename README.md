# Scheme-Compiler
A basic scheme compiler written in C

## Current Status
- ✅ Scanner (Lexical Analysis) - Complete
- ✅ Parser (Syntax Analysis) - Complete  
- 🚧 Analyzer (Semantic Analysis) - In Progress
- ⏳ Code Generator - Not Started

## TODO List

### Phase 1: Build Basic Analyzer (Current Focus)
Goal: Analyze simple expression `(display (+ 1 2))`

#### Step 1: Design Analyzer Interface
- [ ] Create `include/analyzer/analyzer.h` with:
  - [ ] `analyzer` struct (holds symbol table, error state)
  - [ ] `init_analyzer()` function
  - [ ] `analyze(analyzer*, ast_node*)` main entry point
  - [ ] `cleanup_analyzer()` function

#### Step 2: Implement Core Analysis Functions
- [ ] Create `src/analyzer/analyzer.c` with:
  - [ ] `analyze_expression(analyzer*, ast_node*)` - recursively analyze nodes
  - [ ] `analyze_atom(analyzer*, ast_node*)` - validate literals and identifiers
  - [ ] `analyze_function_call(analyzer*, ast_node*)` - validate function calls
  - [ ] Helper function to check if identifier is defined (in symbol table or built-in)

#### Step 3: Type Checking (Simple Version)
- [ ] Implement type inference for atoms:
  - [ ] `TOKEN_DEC` → numeric type
  - [ ] `TOKEN_REAL` → numeric type
  - [ ] `TOKEN_STR_LITERAL` → string type
- [ ] Validate function argument types:
  - [ ] `+` requires numeric arguments
  - [ ] `display` accepts any type
  - [ ] Report type mismatch errors

#### Step 4: Built-in Function Handling
- [ ] Create helper function `is_builtin(token_type)` or `is_builtin(token*)`
- [ ] For now, use current scanner tokens (TOKEN_DISPLAY, TOKEN_PLUS, etc.)
- [ ] Validate correct number of arguments per built-in
- [ ] Validate argument types per built-in

#### Step 5: Testing
- [ ] Test with `(display (+ 1 2))` - should pass
- [ ] Test with `(display (+ "hello" 2))` - should report type error
- [ ] Test with `(+ 1)` - should report arity error (wrong number of args)
- [ ] Test with `(undefined-func 1)` - should report undefined function

### Phase 2: Scanner Refactoring (After Analyzer Works)
Goal: Properly separate keywords from built-in procedures

#### Step 1: Understand Current Problem
- [ ] Document: Built-in procedures (like `+`, `display`) can be redefined in Scheme
- [ ] Document: Special forms (like `if`, `define`) cannot be redefined
- [ ] Current scanner treats built-ins as keywords - this is incorrect!

#### Step 2: Refactor Token Types
- [ ] Keep as keywords (special tokens):
  - [ ] `TOKEN_IF`, `TOKEN_DEFINE`, `TOKEN_LAMBDA`
  - [ ] `TOKEN_LET`, `TOKEN_LET_STAR`, `TOKEN_LETREC`
  - [ ] `TOKEN_QUOTE`, `TOKEN_SET`, `TOKEN_BEGIN`, `TOKEN_COND`
  - [ ] `TOKEN_AND`, `TOKEN_OR` (these do short-circuit evaluation)
- [ ] Change to TOKEN_IDENTIFIER:
  - [ ] `TOKEN_PLUS` → becomes identifier `+`
  - [ ] `TOKEN_MINUS` → becomes identifier `-`
  - [ ] `TOKEN_DISPLAY` → becomes identifier `display`
  - [ ] `TOKEN_CAR`, `TOKEN_CDR`, `TOKEN_CONS`, etc.
  - [ ] All arithmetic operators
  - [ ] All type predicates (`pair?`, `null?`, etc.)

#### Step 3: Update Scanner Implementation
- [ ] Modify `check_keyword()` in `src/scanner/token.c`
  - [ ] Remove built-in procedures from keyword checking
  - [ ] Keep only special forms
- [ ] Test scanner output - built-ins should now be TOKEN_IDENTIFIER

#### Step 4: Pre-populate Global Symbol Table
- [ ] In `init_analyzer()`, add all built-in procedures to global scope:
  - [ ] Arithmetic: `+`, `-`, `*`, `/`, `modulo`, `abs`, `max`, `min`, `sqrt`, `expt`
  - [ ] List ops: `car`, `cdr`, `cons`, `list`, `append`, `reverse`, `length`
  - [ ] Type predicates: `pair?`, `null?`, `number?`, `string?`, `procedure?`
  - [ ] I/O: `display`, `newline`, `read`, `write`
  - [ ] Logic: `not`
  - [ ] Equality: `eq?`, `eqv?`, `equal?`
- [ ] Store type signature information with each built-in

#### Step 5: Update Analyzer for Refactored Scanner
- [ ] Change `is_builtin()` to check symbol table instead of token type
- [ ] Update all analyzer functions that checked specific token types
- [ ] Test that `(display (+ 1 2))` still works correctly

### Phase 3: Expand Analyzer Features
- [ ] Handle special forms:
  - [ ] `define` - add to symbol table
  - [ ] `lambda` - create function scope
  - [ ] `let`/`let*`/`letrec` - create local scopes
  - [ ] `if` - analyze both branches
- [ ] Variable definition tracking
- [ ] Scope management for nested contexts
- [ ] Function arity checking
- [ ] More sophisticated type system

### Phase 4: Code Generation
- [ ] Choose target (bytecode, C code, or assembly)
- [ ] Implement code generator
- [ ] Runtime system for built-in functions

## Architecture Notes

### Proper Separation of Concerns
- **Scanner**: Recognizes lexical structure (keywords, identifiers, numbers, strings)
  - Should only know about SYNTAX (what things look like)
  - Special forms are keywords because they have special syntax
  
- **Analyzer**: Understands meaning of identifiers
  - Should know about SEMANTICS (what things mean)
  - Built-in functions are semantic knowledge, not lexical

### Why This Matters
In Scheme, you can write:
```scheme
(define + -)      ; Redefine + to mean subtraction
(+ 5 3)           ; Returns 2, not 8!
```

But you cannot write:
```scheme
(define if +)     ; ERROR: 'if' is a special form
```

This is why built-ins must be identifiers (looked up in symbol table) while special forms must be keywords (recognized by scanner).

