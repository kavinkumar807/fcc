#ifndef fcc_compiler_h
#define fcc_compiler_h

#include "object.h"
#include "vm.h"

// function declaration for compiler
bool compile(const char* source, Chunk* chunk);

#endif