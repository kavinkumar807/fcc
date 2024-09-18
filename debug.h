#ifndef fcc_debug_h
#define fcc_debug_h

#include "chunk.h"

//function declarations for disassemble functions
void disassembleChunk(Chunk* chunk, const char* name);
int disassembleInstruction(Chunk* chunk, int offset);

#endif