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
    Printf("Expected %d, got %d\n", total_args, argc);
  }
  s2 = dstrtol(argv[2], NULL, 10);
  s = dstrtol(argv[3], NULL, 10);
  sem = dstrtol(argv[1], NULL, 10);

  if (mbox_open(s2) == MBOX_FAIL) {
    Printf("Error in opening S2 in pid %d\n", getpid());
    Exit();
  }
  if (mbox_open(s) == MBOX_FAIL) {
    Printf("Error in opening S in pid %d\n", getpid());
    Exit();
  }
  if (mbox_recv(s2, sizeof(S2_MSG), (void *)message) == MBOX_FAIL) {
    Printf("Error in receiving S2 in pid %d\n", getpid());
    Exit();
  }
  if (mbox_send(s, sizeof(S_MSG), message) != MBOX_SUCCESS) {
    Printf("Error in sending S in pid %d\n", getpid());
    Exit();
  }
  Printf("S molecule made succesfully in pid %d\n", getpid());

  if (mbox_send(s, sizeof(S_MSG), message) != MBOX_SUCCESS) {
    Printf("Error in sending S in pid %d\n", getpid());
    Exit();
  }
  Printf("S molecule made succesfully in pid %d\n", getpid());

  Printf("Reaction_1 (%d): Good bye!!!!\n", getpid());
  semSignalOrDie(sem);
}
