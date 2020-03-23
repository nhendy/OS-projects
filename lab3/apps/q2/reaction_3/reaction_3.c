#include "utils.h"
#include "common.h"
#include "usertraps.h"
#include "misc.h"

void main(int argc, char *argv[]) {
  mbox_t s, o2, so4;
  sem_t sem;
  int total_args, k;
  char message_s[sizeof(S_MSG)];
  char message_o2[sizeof(O2_MSG)];
  char message_so4[sizeof(SO4_MSG)];
  total_args = 5;
  if (argc < total_args) {
    LOG("Too few args in Reaction 3. Exiting...\n");
    Printf("Expected %d, got %d\n", total_args, argc);
  }
  sem = dstrtol(argv[1], NULL, 10);
  s = dstrtol(argv[2], NULL, 10);
  o2 = dstrtol(argv[3], NULL, 10);
  so4 = dstrtol(argv[4], NULL, 10);

  if (mbox_open(s) == MBOX_FAIL) {
    Printf("Error in opening S in pid %d\n", getpid());
    Exit();
  }
  if (mbox_open(o2) == MBOX_FAIL) {
    Printf("Error in opening O2 in pid %d\n", getpid());
    Exit();
  }
  if (mbox_open(so4) == MBOX_FAIL) {
    Printf("Error in opening SO4 in pid %d\n", getpid());
    Exit();
  }
  if (mbox_recv(s, sizeof(S_MSG), message_s) == MBOX_FAIL) {
    Printf("Error in recieving S in pid %d\n", getpid());
    Exit();
  }
  for (k = 0; k < 2; k++) {
    if (mbox_recv(o2, sizeof(O2_MSG), message_o2) == MBOX_FAIL) {
      Printf("Error in sending O2 in pid %d\n", getpid());
      Exit();
    }
  }
  if (mbox_send(so4, sizeof(SO4_MSG), message_so4) != MBOX_SUCCESS) {
    Printf("Error in sending SO4 in pid %d\n", getpid());
    Exit();
  }
  Printf("SO4 molecule made succesfully in pid %d\n", getpid());

  Printf("Reaction_3 (%d): Good bye!!!!\n", getpid());
  semSignalOrDie(sem);
}
