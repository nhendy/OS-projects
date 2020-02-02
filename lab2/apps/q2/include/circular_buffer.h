#ifndef __CIRCULAR_BUFFER__
#define __CIRCULAR_BUFFER__

#include "common.h"

typedef struct CircularBuffer_t {
  int write_index;
  int read_index;
  char buffer[BUFFER_MAX_SIZE];
} CircularBuffer;
// Initialize circular buffer
int initCircularBuffer(CircularBuffer* const buffer);
// Writes a char to the buffer
int write(CircularBuffer* const buffer, const char* const data);
// Reads a char from the buffer
int read(CircularBuffer* const buffer, char* const result);
#endif
