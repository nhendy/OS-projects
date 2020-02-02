#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"
#include "common.h"
#include "utils.h"
#include "circular_buffer.h"

ConsumerProducerContext decodeArgs(const int argc,
                                   const char* const* const argv) {
  ConsumerProducerContext ctxt;
  uint32 circ_buff_handle;
  sem_t producer_sem;
  sem_t consumer_sem;
  sem_t all_processes_done_sem;
  CircularBuffer* circ_buffer_ptr;

  if (argc != NUM_CMDLINE_ARGS) {
    Printf("Wrong number of args\n");
    Exit();
  }

  // Decode cmdline args
  ctxt.circ_buff_handle = dstrtol(argv[1], NULL, 10);
  ctxt.producer_sem = dstrtol(argv[2], NULL, 10);
  ctxt.consumer_sem = dstrtol(argv[3], NULL, 10);
  ctxt.all_processes_done_sem = dstrtol(argv[4], NULL, 10);

  // Attach to shared memory
  if ((ctxt.circ_buffer_ptr = shmat(circ_buff_handle)) == NULL) {
    Printf("Failed to attach to memory\n");
    Exit();
  }
  return ctxt;
}

void cleanAndSignal(const ConsumerProducerContext ctxt) {
  if (sem_signal(ctxt.all_processes_done_sem) != SYNC_SUCCESS) {
    Printf("Failed to sem_signal all_processes_done_sem");
    Exit();
  }
}
