#include "circular_buffer.h"
#include "common.h"
#include "misc.h"
#include "usertraps.h"

// Helper function
void memsetZero(char* const data, int n) {
  int i;
  for (i = 0; i < n; ++i) {
    data[i] = 0;
  }
}

// Caller handles memory allocation
int initCircularBuffer(CircularBuffer* const buffer) {
  buffer->read_index = 0;
  buffer->write_index = 0;
  memsetZero(buffer->buffer, BUFFER_MAX_SIZE);
  return TRUE;
}

// Write one char into the `buffer`.
int write(CircularBuffer* const buffer, const char* const data) {
  int buffer_size = sizeof(buffer->buffer) / sizeof(buffer->buffer[0]);
  if ((buffer->write_index + 1) % buffer_size == buffer->read_index) {
    Printf("BUFFER IS FULLL!!! ERROR");
    return FALSE;
  }
  buffer->buffer[buffer->write_index] = data[0];
  buffer->write_index = (buffer->write_index + 1) % buffer_size;
  return TRUE;
}

// Read one char from `buffer` and place it where `result` is pointing to.
int read(CircularBuffer* const buffer, char* const result) {
  int buffer_size = sizeof(buffer->buffer) / sizeof(buffer->buffer[0]);
  if (buffer->read_index == buffer->write_index) {
    Printf("BUFFER IS EMPTY!!! ERROR");
    return FALSE;
  }
  *result = buffer->buffer[buffer->read_index];
  buffer->read_index = (buffer->read_index + 1) % buffer_size;
  return TRUE;
}
