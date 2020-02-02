#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"
#include "common.h"
#include "utils.h"
#include "circular_buffer.h"

ConsumerProducerContext decodeArgs(const int argc,
                                   const char* const* const argv) {
  ConsumerProducerContext ctxt;
  if (argc != NUM_CMDLINE_ARGS) {
    LOG("Wrong number of args.");
    Printf("Got %d. Expected %d\n", argc, NUM_CMDLINE_ARGS);
    Exit();
  }

  // Decode cmdline args
  ctxt.circ_buff_handle = dstrtol(argv[1], NULL, 10);
  ctxt.producer_sem = dstrtol(argv[2], NULL, 10);
  ctxt.consumer_sem = dstrtol(argv[3], NULL, 10);
  ctxt.all_processes_done_sem = dstrtol(argv[4], NULL, 10);
  ctxt.shared_mu = dstrtol(argv[5], NULL, 10);

  // Attach to shared memory
  if ((ctxt.circ_buffer_ptr = (CircularBuffer*)shmat(ctxt.circ_buff_handle)) ==
      NULL) {
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

void lockAcquireOrDie(lock_t mu) {
  if (lock_acquire(mu) == SYNC_FAIL) {
    Printf("Weird. Couldn't lock_acquire lock: %d, pid: %d\n", mu, getpid());
    Exit();
  }
}

void lockReleaseOrDie(lock_t mu) {
  if (lock_release(mu) == SYNC_FAIL) {
    Printf("Weird. Couldn't lock_release lock: %d, pid: %d\n", mu, getpid());
    Exit();
  }
}

void semSignalOrDie(sem_t sem) {
  if (sem_signal(sem) == SYNC_FAIL) {
    Printf("Weird. Couldn't sem_signal sem: %d, pid: %d\n", sem, getpid());
    Exit();
  }
}

void semWaitOrDie(sem_t sem) {
  if (sem_wait(sem) == SYNC_FAIL) {
    Printf("Weird. Couldn't sem_wait sem: %d, pid: %d\n", sem, getpid());
    Exit();
  }
}
