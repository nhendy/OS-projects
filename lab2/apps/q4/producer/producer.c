#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"
#include "circular_buffer.h"

void main(int argc, char *argv) {

  CircularBuffer *circ_buff_ptr;
  uint32 h_mem;

  lock_t buffer_lock;
  cond_t full;
  cond_t empty;
  sem_t sem_proc;
  int i = 0;
  int buffer_size;

  if (argc != NUM_CMDLINE_ARGS) {
    LOG("Incorrect number of arguments");
    Printf("Got %d.Expected %d\n", argc, NUM_CMDLINE_ARGS);
    Exit();
  }
  h_mem = dstrol(argv[1], NULL, 10);
  sem_proc = dstrol(argv[2], NULL, 10);
  buffer_lock = dstrol(argv[3], NULL, 10);
  full = dstrol(argv[4], NULL, 10);
  empty = dstrol(argv[5], NULL, 10);

  if (circ_buff_ptr = ((CircularBuffer *)shmat(h_mem)) == NULL) {
    Printf("Error in mapping shared memory page");
    Exit();
  }

  for (i = 0; i <= dstrlen(BUFFER_MAX_SIZE); ++i) {
    if (lock_acquire(buffer_lock) == SYNC_FAIL) {
      Printf("Could not aquire lock: %d, pid: %d", buffer_lock);
      Exit();
    }

    while (circ_buff_ptr->read_index =
               (circ_buff_ptr->write_index + 1 % BUFFER_MAX_SIZE)) {
      cond_wait(empty);
    }

    circ_buff_ptr->buffer[circ_buff_ptr->write_index] = HELLO_WORLD_STR[i];
    circ_buff_ptr->write_index =
        (circ_buff_ptr->write_index + 1) % BUFFER_MAX_SIZE;
    Printf("%c added from buffer, pid: %d", HELLO_WORLD_STR[i], getpid());
    cond_signal(full);
    if (lock_release(buffer_lock) == SYNC_FAIL) {
      Printf("Could not release lock: %d, pid: %d", buffer_lock, getpid());
      Exit();
    }
    Printf("consumer:PID %d is complete.\n", getpid());
    if (sem_signal(sem_proc) == SYNC_FAIL) {
      Printf('Bad semephore sem_proc %d in ', sem_proc);
      Exit();
    }
  }
}
