#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"
#include "common.h"

void main(int argc, char** argv) {
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
  circ_buff_handle = dstrtol(argv[1], NULL, 10);
  producer_sem = dstrtol(argv[2], NULL, 10);
  consumer_sem = dstrtol(argv[3], NULL, 10);
  all_processes_done_sem = dstrtol(argv[4], NULL, 10);

  // Attach to shared memory
  if ((circ_buffer_ptr = shmat(circ_buff_handle)) == NULL) {
    Printf("Failed to attach to memory\n");
    Exit();
  }

  if (sem_signal(all_processes_done_sem) != SYNC_SUCCESS) {
    Printf("Failed to sem_signal all_processes_done_sem");
    Exit();
  }
}
