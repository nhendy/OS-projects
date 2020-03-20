#include "utils.h"
#include "common.h"
#include "usertraps.h"
#include "misc.h"

void main(int argc, char *argv[]) {
  mbox_t s, o2, so4;
  sem_t sem;
  int total_args, k;
  char message_s[sizeof(S_MSG)];
  char message_O2[sizeof(O2_MSG)];
  total_args = 5;
  if (argc < total_args) {
    LOG("Too few args in Reactions. Exiting...\n");
    Printf("Expected %d, got %d\n", 3, total_args);
  }
  s = dstrtol(argv[1], NULL, 10);
  o2 = dstrtol(argv[2], NULL, 10);
  so4 = dstrtol(argv[3], NULL, 10);
  sem = dstrtol(argv[4], NULL, 10);
  if (mbox_recv(s, sizeof(s), message_s) != MBOX_SUCCESS) {
    LOG("Bad s receive");
    Exit();
  }
  for (k = 0; k < 2; k++) {
    if (mbox_send(o2, sizeof(o2), O2_MSG) != MBOX_SUCCESS) {
      LOG("o2 send failure");
      Exit();
    }
  }
  if (mbox_send(so4, sizeof(so4), SO4_MSG) != MBOX_SUCCESS) {
    LOG("c2 send failure");
    Exit();
  }
  semSignalOrDie(sem);
}
