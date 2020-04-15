#include "usertraps.h"
#include "misc.h"

#define HELLO_WORLD "hello_world.dlx.obj"
#define TEST_SIX "test_six.dlx.obj"

void main(int argc, char *argv[]) {
  int num_procs = 0;        // Used to store number of processes to create
  int i;                    // Loop index variable
  sem_t s_procs_completed;  // Semaphore used to wait until all spawned
                            // processes have completed
  char s_procs_completed_str[10];  // Used as command-line argument to pass
                                   // page_mapped handle to new processes

  if (argc != 2) {
    Printf("Usage: %s <number of hello world processes to create>\n", argv[0]);
    Exit();
  }

  num_procs = dstrtol(argv[1], NULL, 10);

  if (num_procs != 30) {
    Printf("makeprocs (%d): Creating %d  processes\n", getpid(), num_procs);

    if ((s_procs_completed = sem_create(0)) == SYNC_FAIL) {
      Printf("makeprocs (%d): Bad sem_create\n", getpid());
      Exit();
    }
    ditoa(s_procs_completed, s_procs_completed_str);
    // Create Hello World processes
    Printf(
        "----------------------------------------------------------------------"
        "--"
        "-------------\n");
    Printf(
        "makeprocs (%d): Creating %d hello world's in a row, but only one runs "
        "at a time\n",
        getpid(), num_procs);
    for (i = 0; i < num_procs; i++) {
      Printf("makeprocs (%d): Creating hello world #%d\n", getpid(), i);
      process_create(HELLO_WORLD, s_procs_completed_str, NULL);
      if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
        Printf("Bad semaphore s_procs_completed (%d) in %s\n",
               s_procs_completed, argv[0]);
        Exit();
      }
    }
  } else if (num_procs == 30) {
    Printf("makeprocs (%d): Creating %d  processes\n", getpid(), num_procs);
    Printf(
        "----------------------------------------------------------------------"
        "--"
        "-------------\n");
    Printf("makeprocs (%d): running Test 6. Spawning %d processes\n", getpid(),
           num_procs);

    if ((s_procs_completed = sem_create(-(num_procs - 1))) == SYNC_FAIL) {
      Printf("makeprocs (%d): Bad sem_create\n", getpid());
      Exit();
    }
    ditoa(s_procs_completed, s_procs_completed_str);
    for (i = 0; i < num_procs; i++) {
      Printf("makeprocs (%d): Creating process #%d\n", getpid(), i);
      process_create(TEST_SIX, s_procs_completed_str, NULL);
    }
    if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
      Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed,
             argv[0]);
      Exit();
    }
  }

  Printf(
      "------------------------------------------------------------------------"
      "-------------\n");
  Printf(
      "makeprocs (%d): All other processes completed, exiting main process.\n",
      getpid());
}
