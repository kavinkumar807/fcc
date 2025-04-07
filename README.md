# fcc

## C compiler for Fein Language

This project implements a compiler and virtual machine for a dynamic language called "Fein". The implementation is written in C and follows many principles outlined in the "Crafting Interpreters" book by Robert Nystrom.

### 1. Project Setup

The project is structured into several C source (`.c`) and header (`.h`) files, each responsible for a specific component:

*   **`main.c`**: Entry point, handles command-line arguments for REPL or file execution.
*   **`scanner.c`/`.h`**: Lexical analysis (tokenization) of the source code.
*   **`compiler.c`/`.h`**: Parses tokens and compiles source code directly into bytecode.
*   **`chunk.c`/`.h`**: Data structure (`Chunk`) to store bytecode and associated data (like constants and line numbers).
*   **`vm.c`/`.h`**: The stack-based virtual machine that executes the bytecode.
*   **`value.c`/`.h`**: Defines the `Value` type system used by the VM (numbers, booleans, nil, objects).
*   **`object.c`/`.h`**: Handles heap-allocated objects (currently strings).
*   **`memory.c`/`.h`**: Custom memory management utilities (allocation, deallocation, resizing arrays).
*   **`table.c`/`.h`**: Hash table implementation used for global variables and string interning.
*   **`debug.c`/`.h`**: Utilities for disassembling bytecode chunks for debugging.
*   **`common.h`**: Common definitions and includes used across the project.

The project can be built using a standard C compiler (like GCC or Clang). It supports two modes:
1.  **REPL:** Running `fcc` with no arguments starts an interactive Read-Eval-Print Loop.
2.  **File Execution:** Running `fcc <path_to_file>` reads, compiles, and executes the script from the specified file.

### 2. Single Pass Compiler

The compiler (`compiler.c`) operates in a **single pass**. This means it reads the source code (as a stream of tokens from the scanner) and generates executable bytecode directly, without building an intermediate representation like an Abstract Syntax Tree (AST) first.

*   **Process:** The scanner (`scanner.c`) produces tokens one by one. The compiler uses a recursive descent parser. As the parser recognizes grammatical structures (expressions, statements), it immediately emits the corresponding bytecode instructions (`emitByte`, `emitBytes`, `emitConstant`) into the current `Chunk`.
*   **Efficiency:** This approach can be faster and use less memory than multi-pass compilers, as it avoids the overhead of constructing and traversing an entire AST.
*   **Limitations:** Single-pass compilation can make implementing features like forward references or complex optimizations more challenging.

### 3. Grammar

The language's grammar is implicitly defined by the recursive descent parser implemented in `compiler.c`. Key grammar elements handled include:

*   **Literals:** Numbers (`number`), Strings (`string`), Booleans (`literal` for `true`, `false`), `nil` (`literal`).
*   **Expressions:** Grouping (`grouping`), Unary operators (`unary`: `!`, `-`), Binary operators (`binary`: `+`, `-`, `*`, `/`, `==`, `!=`, `<`, `>`, `<=`, `>=`), Logical operators (`and_`, `or_`). Operator precedence is handled by the `Precedence` enum and `parsePrecedence` function.
*   **Variables:** Declaration (`varDeclaration`), Assignment, Access (`variable`, `namedVariable`). Both global and local scopes are supported.
*   **Statements:** Expression statements (`expressionStatement`), Print statements (`printStatement`), Block statements (`block`, `{ ... }`), If statements (`ifStatement`), While loops (`whileStatement`), For loops (`forStatement`).
*   **Declarations:** Variable declarations (`varDeclaration`).

The parser follows rules like:

declaration -> varDecl | statement ;
statement -> exprStmt | forStmt | ifStmt | printStmt | whileStmt | block ;
// (And many more implicit rules within the parsing functions)


### 4. No Intermediate AST

**Crucially, this compiler does *not* build an Abstract Syntax Tree (AST).** As mentioned in the "Single Pass Compiler" section, the parsing functions (`expression`, `statement`, `binary`, `unary`, etc.) directly call bytecode emission functions (`emitByte`, `emitBytes`, etc.) as soon grammar rules are recognized. The program structure exists only transiently within the parser's function call stack during compilation.

### 5. Hash Function used FNV-1a

String hashing, essential for the hash table implementation (see below), uses the **FNV-1a algorithm**. This is implemented in the `hashString` function within `object.c`.

```c
// object.c
static uint32_t hashString(const char* key, int length) {
    uint32_t hash = 2166136261u; // FNV offset basis
    for(int i = 0; i < length; i++) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;       // FNV prime
    }
    return hash;
}
```

### 6. Hash Tables
A custom hash table implementation (`table.c`, `table.h`) is used for managing:

1. **Global Variables**: The `vm.globals` table stores global variable names (as `ObjString*` keys) and their corresponding Values.

2. **String Interning**: The `vm.strings` table stores unique `ObjString*` instances to ensure that identical string literals occupy the same memory location.

3. **Implementation**: It uses open addressing with linear probing for collision resolution.

4. **Load Factor**: The table automatically resizes (grows) when the load factor exceeds TABLE_MAX_LOAD (0.75) to maintain performance.

5. **Tombstones**: Deleted entries are marked with a special tombstone value (BOOL_VAL(true) in the value field with a NULL key) to ensure probes correctly find entries beyond deleted slots.

### 7. String Interning for All Strings

All strings within the VM are `interned`. When a string is created (either from a literal in the source code or dynamically, e.g., via concatenation):

1. Its hash is computed (using FNV-1a).
2. The vm.strings hash table is checked (tableFindString) to see if an identical string (same characters, length, and hash) already exists.
3. If found, a pointer to the existing ObjString is returned.
4. If not found, a new ObjString is allocated on the heap, stored in the vm.strings table (allocateString), and a pointer to the new object is returned.

This ensures that any two identical strings in the program point to the exact same object in memory. This makes string comparison very fast (just a pointer comparison via valuesEqual for VAL_OBJ) and reduces overall memory usage.

### 8. Stack Based VM
The core execution engine is a **stack-based Virtual Machine (VM)** implemented in `vm.c` and `vm.h`.

- **Stack**: The VM uses a fixed-size array (`vm.stack`) as its operand stack. Instructions push values onto the stack, operate on the top values, and pop results back onto the stack. `vm.stackTop` points to the next available slot.

- **Instruction Pointer** : `vm.ip` (instruction pointer) points to the next bytecode instruction in the `Chunk` to be executed.

- **Execution Loop**: The `run` function contains the main loop that fetches an opcode, decodes it, performs the corresponding action (often involving stack manipulation), and repeats until an `OP_RETURN` instruction is encountered or an error occurs.

### 9. Heap Storage
While the VM uses a stack for temporary values during computation, objects (currently only ObjString) are allocated on the heap.

- **Allocation**: The memory management functions (memory.c, memory.h, especially reallocate) are used to request memory from the system heap. ALLOCATE_OBJ in object.c allocates memory for new objects.

- **Object Linking**: All allocated objects are tracked via a linked list (vm.objects points to the head). Each Obj has a next pointer.

- **Garbage Collection (Implicit)**: Currently, there's no garbage collector. All allocated objects are freed only when the VM shuts down (freeVM calls freeObjects).

### 10. OpCode Table

The VM executes bytecode instructions defined by the `OpCode` enum in `chunk.h`. Here's a summary of the opcodes:

| OpCode             | Operands        | Description                                                                  |
| :----------------- | :-------------- | :--------------------------------------------------------------------------- |
| `OP_CONSTANT`      | `uint8_t` index | Pushes a constant value from the chunk's constant pool onto the stack.         |
| `OP_NIL`           |                 | Pushes `nil` onto the stack.                                                 |
| `OP_TRUE`          |                 | Pushes `true` onto the stack.                                                |
| `OP_FALSE`         |                 | Pushes `false` onto the stack.                                               |
| `OP_POP`           |                 | Pops the top value from the stack.                                           |
| `OP_GET_LOCAL`     | `uint8_t` slot  | Pushes the value from the specified local variable slot onto the stack.      |
| `OP_SET_LOCAL`     | `uint8_t` slot  | Sets the specified local variable slot to the value at the top of the stack (doesn't pop). |
| `OP_GET_GLOBAL`    | `uint8_t` index | Gets a global variable's value using its name (string constant at index) and pushes it. |
| `OP_DEFINE_GLOBAL` | `uint8_t` index | Defines a new global variable using its name (string constant at index) and the value on top of the stack (pops value). |
| `OP_SET_GLOBAL`    | `uint8_t` index | Sets an existing global variable's value using its name (string constant at index) to the value on top of the stack (doesn't pop). |
| `OP_EQUAL`         |                 | Pops two values, pushes `true` if they are equal, `false` otherwise.          |
| `OP_GREATER`       |                 | Pops two numbers, pushes boolean result of `a > b`.                          |
| `OP_LESS`          |                 | Pops two numbers, pushes boolean result of `a < b`.                          |
| `OP_ADD`           |                 | Pops two numbers or two strings, pushes their sum or concatenation.          |
| `OP_SUBTRACT`      |                 | Pops two numbers, pushes their difference (`a - b`).                         |
| `OP_MULTIPLY`      |                 | Pops two numbers, pushes their product (`a * b`).                            |
| `OP_DIVIDE`        |                 | Pops two numbers, pushes their quotient (`a / b`).                           |
| `OP_NOT`           |                 | Pops a value, pushes its boolean negation (truthiness).                      |
| `OP_NEGATE`        |                 | Pops a number, pushes its negation (`-a`).                                   |
| `OP_PRINT`         |                 | Pops a value and prints it to the console.                                   |
| `OP_JUMP`          | `uint16_t` off  | Unconditionally jumps the instruction pointer forward by `offset`.             |
| `OP_JUMP_IF_FALSE` | `uint16_t` off  | Pops a value; if it's falsey, jumps the instruction pointer forward by `offset`. |
| `OP_LOOP`          | `uint16_t` off  | Unconditionally jumps the instruction pointer backward by `offset`.            |
| `OP_RETURN`        |                 | Pops the final value (currently unused) and exits the VM execution loop.     |