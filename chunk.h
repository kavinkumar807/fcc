#ifndef fcc_chunk_h
#define fcc_chunk_h

#include "common.h"
#include "value.h"

/**
 * Enum for operation code
 */
typedef enum{
    OP_CONSTANT,
    OP_RETURN,
} OpCode;

/**
 * Structure to hold the instruction chunk
 * 
 * capacity "number of elements allocated in the array"
 * count "how many allocated elements are in use"
 * constants "chunk constants"
 * code "byte_code array",
 * lines "line of source file"
 */
typedef struct {
    int count;
    int capacity;
    uint8_t* code;
    int* lines;
    ValueArray constants;
    
} Chunk;

// function declarations for chunk functions
void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
int addConstant(Chunk* chunk, Value value);

#endif
