#include "utils.h"
#include "common.h"
#include "usertraps.h"
#include "misc.h"
// TODO" create global msg_size variable
void main(int argc, char **argv) {
  mbox_t s2, s;
  sem_t sem;
  int total_args = 4;
  char message[sizeof(S2_MSG)];
  if (argc < total_args) {
    LOG("Two few args in Reaction 1. Exiting...\n");
    Printf("Expected %d, got %d\n", 3, total_args);
  }
  s2 = dstrtol(argv[2], NULL, 10);
  s = dstrtol(argv[3], NULL, 10);
  sem = dstrtol(argv[1], NULL, 10);

  if (mbox_open(s2) == MBOX_FAIL) {
    LOG("Error in opening s2 mbox");
    Exit();
  }
  Printf("s2: %d, s: %d\n", s2, s);
  if (mbox_open(s) == MBOX_FAIL) {
    LOG("Error in opening s mbox\n");
    Exit();
  }
  if (mbox_recv(s2, sizeof(S2_MSG), (void *)message) != MBOX_SUCCESS) {
    LOG("S2 receive failure\n");
    Exit();
  }
  if (mbox_send(s, sizeof(S_MSG), message) != MBOX_SUCCESS) {
    LOG("S send failure\n");
    Exit();
  }
  Printf("S molecule made\n");
  if (mbox_send(s, sizeof(S_MSG), message) != MBOX_SUCCESS) {
    LOG("S send failure\n");
    Exit();
  }
  Printf("S molecule made\n");
  Printf("Reaction 1 is over. PID: %d!!\n", getpid());
  semSignalOrDie(sem);
}
