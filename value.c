#include <stdio.h>

#include "memory.h"
#include "value.h"

/**
 * function to initialize value array
 */
void initValueArray(ValueArray* array) {
  array->values = NULL;
  array->capacity = 0;
  array->count = 0;
}

/**
 * function to store value array parallel to byte_code array
 */
void writeValueArray(ValueArray* array, Value value) {
  if (array->capacity < array->count + 1) {
    int oldCapacity = array->capacity;
    array->capacity = GROW_CAPACITY(oldCapacity);
    array->values = GROW_ARRAY(Value, array->values,
                               oldCapacity, array->capacity);
  }

  array->values[array->count] = value;
  array->count++;
}

/**
 * function to free value array
 */
void freeValueArray(ValueArray* array) {
  FREE_ARRAY(Value, array->values, array->capacity);
  initValueArray(array);
}

/**
 * function to print value
 */
void printValue(Value value) {
  printf("%g", value);
}
