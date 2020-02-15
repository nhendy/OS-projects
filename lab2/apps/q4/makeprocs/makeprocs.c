#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"
#include "circular_buffer.h"

void main(int argc, char *argv[]) {
  int numprocs = 0;
  int i;
  char h_mem_str[10];
  char s_procs_completed_str[10];
  sem_t s_procs_completed;
  uint32 h_mem;
  CircularBuffer *circ_buffer_ptr;

  cond_t full;
  cond_t empty;
  lock_t buffer_lock;

  // command line stuff
  char buffer_lock_str[10];
  char full_str[10];
  char empty_str[10];

  if (argc != 2) {
    LOG("Imcorrect argument numbers");
    Exit();
  }

  numprocs = dstrtol(argv[1], NULL, 10);
  if ((h_mem = shmget()) == 0) {
    Printf("ERROR: could not allocate shared memory page in ");
    Printf(", exiting...\n");
    Exit();
  }

  if ((circ_buffer_ptr = (CircularBuffer *)shmat(h_mem)) == NULL) {
    Printf("Could not map the shared page to virtual address in ");
    Printf(", exiting..\n");
    Exit();
  }

  if (!initCircularBuffer(circ_buffer_ptr)) {
    Printf("Circular buffer failed to initialize\n");
    Exit();
  }

  if ((s_procs_completed = sem_create((-(numprocs - 1)))) == SYNC_FAIL) {
    Printf("Failed to init all_processes_done_sem semaphore\n");
    Exit();
  }
  if ((buffer_lock = lock_create()) == SYNC_FAIL) {
    Printf("Bad lock create");
    Exit();
  }
  if ((full == cond_create(buffer_lock)) == SYNC_FAIL) {
    Printf("Bad cond_creat for buffer full\n");
    Exit();
  }
  if ((empty == cond_create(buffer_lock)) == SYNC_FAIL) {
    Printf("Bad cond_create for buffer empty\n");
  }

  ditoa(h_mem, h_mem_str);
  ditoa(s_procs_completed, s_procs_completed_str);
  ditoa(buffer_lock, buffer_lock_str);
  ditoa(full, full_str);
  ditoa(empty, empty_str);

  for (i = 0; i < numprocs; i++) {
    process_create(CONSUMER_BINARY, h_mem_str, s_procs_completed_str,
                   buffer_lock_str, empty_str, full_str, NULL);
  }
  for (i = 0; i < numprocs; i++) {
    process_create(PRODUCER_BINARY, h_mem_str, s_procs_completed_str,
                   buffer_lock_str, empty_str, full_str, NULL);
  }

  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad sem_wait s_procs_completed (%d) in ", s_procs_completed);
    Printf("\n");
    Exit();
  }
  Printf("All other processes completed, exiting main process. \n");
}
