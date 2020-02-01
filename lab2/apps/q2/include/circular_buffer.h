#ifndef __CIRCULAR_BUFFER__
#define __CIRCULAR_BUFFER__

#include "common.h"

typedef struct {
  int write_index;
  int read_index;
  char buffer[BUFFER_MAX_SIZE];
} circular_buffer;

// Initialize circular buffer
int init_circular_buffer(circular_buffer* const buffer);
// Writes a char to the buffer
int write(circular_buffer* const buffer, const char* const data);
// Reads a char from the buffer
int read(circular_buffer* const buffer, char* const result);
#endif
