#include "circular_buffer.h"
#include "lab2-api.h"
#include "misc.h"
#include "usertraps.h"

void main(int argc, char *argv[]) {
  CircularBuffer *circ_buff_ptr;
  uint32 h_mem;

  lock_t buffer_lock;
  cond_t full;
  cond_t empty;
  sem_t sem_proc;
  int i = 0;

  Printf("Producer %d is UP!!!\n", getpid());

  if (argc != NUM_CMDLINE_ARGS) {
    LOG("Incorrect number of arguments");
    Printf("Got %d.Expected %d\n", argc, NUM_CMDLINE_ARGS);
    Exit();
  }
  h_mem = dstrtol(argv[1], NULL, 10);
  sem_proc = dstrtol(argv[2], NULL, 10);
  buffer_lock = dstrtol(argv[3], NULL, 10);
  full = dstrtol(argv[4], NULL, 10);
  empty = dstrtol(argv[5], NULL, 10);

  if ((circ_buff_ptr = (CircularBuffer *)shmat(h_mem)) == NULL) {
    Printf("Error in mapping shared memory page\n");
    Exit();
  }

  for (i = 0; i < dstrlen(HELLO_WORLD_STR); ++i) {
    lockAcquireOrDie(buffer_lock);

    while (circ_buff_ptr->read_index ==
           (circ_buff_ptr->write_index + 1 % BUFFER_MAX_SIZE)) {
      condWaitOrDie(empty);
    }

    circ_buff_ptr->buffer[circ_buff_ptr->write_index] = HELLO_WORLD_STR[i];
    circ_buff_ptr->write_index =
        (circ_buff_ptr->write_index + 1) % BUFFER_MAX_SIZE;
    Printf("Producer %d inserted %c\n", getpid(), HELLO_WORLD_STR[i]);
    condSignalOrDie(full);
    lockReleaseOrDie(buffer_lock);
  }
  Printf("producer:PID %d is complete.\n", getpid());
  if (sem_signal(sem_proc) == SYNC_FAIL) {
    Printf("Bad semephore sem_proc %d in %d\n.", sem_proc, getpid());
    Exit();
  }
}
