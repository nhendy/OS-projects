#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"
#include "circular_buffer.h"

#define INVALID_NUM_PROCESSES -1

void main(int argc, char** argv) {
  int num_consumers = INVALID_NUM_PROCESSES;
  int num_producers = INVALID_NUM_PROCESSES;
  int buff_size;
  CircularBuffer* circ_buffer_ptr;
  int i;
  uint32 circ_buff_handle;
  sem_t producer_sem;
  sem_t consumer_sem;
  sem_t all_processes_done_sem;
  // Args for child processes
  char all_processes_done_sem_str[10];
  char producer_sem_str[10];
  char consumer_sem_str[10];
  char circ_buff_handle_str[10];

  if (argc != 2) {
    Printf("Wrong number of args\n");
    Exit();
  }

  num_producers = dstrtol(argv[1], NULL, 10);
  num_consumers = dstrtol(argv[1], NULL, 10);

  if ((circ_buff_handle = shmget()) == 0) {
    Printf("Failed to initialize shared memory\n");
    Exit();
  }

  if ((circ_buffer_ptr = shmat(circ_buff_handle)) == NULL) {
    Printf("Failed to attach to shared memory segment\n");
    Exit();
  }

  if (!initCircularBuffer(circ_buffer_ptr)) {
    Printf("Failed to init circular buffer\n");
    Exit();
  }

  buff_size =
      sizeof(circ_buffer_ptr->buffer) / sizeof(circ_buffer_ptr->buffer[0]);

  if ((producer_sem = sem_create(buff_size)) == SYNC_FAIL) {
    Printf("Failed to init producer semaphore\n");
    Exit();
  }

  if ((consumer_sem = sem_create(0)) == SYNC_FAIL) {
    Printf("Failed to init consumer semaphore\n");
    Exit();
  }

  if ((all_processes_done_sem =
           sem_create(-(num_producers + num_consumers - 1))) == SYNC_FAIL) {
    Printf("Failed to init all_processes_done_sem semaphore\n");
    Exit();
  }

  // Convert all semaphores and handle to str.
  ditoa(circ_buff_handle, circ_buff_handle_str);
  ditoa(producer_sem, producer_sem_str);
  ditoa(consumer_sem, consumer_sem_str);
  ditoa(all_processes_done_sem, all_processes_done_sem_str);

  for (i = 0; i < num_consumers; ++i) {
    process_create(CONSUMER_BINARY, circ_buff_handle_str, producer_sem_str,
                   consumer_sem, all_processes_done_sem, NULL);
    Printf("Process %d created\n", i);
  }

  for (i = 0; i < num_producers; ++i) {
    process_create(PRODUCER_BINARY, circ_buff_handle_str, producer_sem_str,
                   consumer_sem, all_processes_done_sem, NULL);
    Printf("Process %d created\n", i);
  }

  if (sem_wait(all_processes_done_sem) != SYNC_SUCCESS) {
    Printf("Failed to sem_wait on all_processes_done_sem\n");
    Exit();
  }

  Printf("All done! Ciao\n");
}
