#include "utils.h"
#include "common.h"
#include "usertraps.h"
#include "misc.h"
// TODO" create global msg_size variable
void main(int argc, char **argv) {
  mbox_t s2, s;
  sem_t sem;
  int total_args = 4 char message[sizeof(S2_MSG)];
  if (argc < total_args) {
    LOG("Two few args in Reactions. Exiting...\n");
    Printf("Expected %d, got %d\n", 3, total_args);
  }
  s2 = dstrtol(argv[1]);
  s = dstrtol(argv[2]);
  sem = dstrtol(argv[3]);

  if (mbox_recv(s2, sizeof(s2), (char *)message) != MBOX_SUCCESS) {
    LOG("S2 receive failure");
    Exit();
  }
  if (mbox_send(s, sizeof(s), S_MSG) != MBOX_SUCCESS) {
    LOG("S send failure");
    Exit();
  }
  if (mbox_send(s, sizeof(s), S_MSG) != MBOX_SUCCESS) {
    LOG("S send failure");
    Exit();
  }

  if (semSignalOrDie(sem) != SYNC_SUCCESS) {
    LOG("Bad semaphore %d in %d", sem, argv[0]);
    Eixt();
  }
}
