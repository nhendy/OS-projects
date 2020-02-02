#ifndef __COMMON_HEADER__
#define __COMMON_HEADER__
#include "misc.h"
#include "lab2-api.h"

// Forward declaration to avoid circular dependency
struct CircularBuffer_t;

// TODO:(nhendy) refactor decoding code into this struct
typedef struct {
  sem_t producer_sem;
  sem_t consumer_sem;
  sem_t all_processes_done_sem;
  uint32 circ_buff_handle;
  struct CircularBuffer_t* circ_buffer_ptr;
} ConsumerProducerContext;
// Consumer and producer binaries
#define CONSUMER_BINARY "consumer.dlx.obj"
#define PRODUCER_BINARY "producer.dlx.obj"
#define NUM_CMDLINE_ARGS 5
// Macros for test truthy/falsey return values
#define FALSE -10000;
#define TRUE 9999;
// Max circular buffer size. Add one extre slot
// to be used to check fullness.
#define BUFFER_MAX_SIZE sizeof("Hello world") + 1
#endif
