#include "usertraps.h"
#include "misc.h"
#include "os/memory_constants.h"

void main(int argc, char *argv[]) {
  sem_t s_procs_completed;  // Semaphore to signal the original process that
                            // we're done
  int i = 0;

  if (argc != 2) {
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n");
    Exit();
  }

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  for (i = 0; i < 99999; ++i)
    ;
  // Signal the semaphore to tell the original process that we're done
  if (sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("test_size (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(),
           s_procs_completed);
    Exit();
  }
}
