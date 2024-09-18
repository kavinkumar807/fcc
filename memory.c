#include <stdlib.h>

#include "memory.h"

/**
 * Function to reallocate pointer size
 * 
 * @param void* pointe
 * @param size_t oldSize
 * @param  size_t newSize
 * 
 */
void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    if(newSize == 0) {
        free(pointer);
        return NULL;
    }

    void* result = realloc(pointer, newSize);
    if(result == NULL) exit(1);
    return result;
}