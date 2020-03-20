#include "utils.h"
#include "common.h"
#include "usertraps.h"
#include "misc.h"

void main(int argc, char *argv[]) {

  mbox_t co;
  sem_t sem;
  int total_args, k;
  char message_s2[sizeof(S2_MSG)];
  total_args = 2;
  if (argc < total_args) {
    LOG("Too few args in Reactions. Exiting...\n");
    Printf("Expected %d, got %d\n", 3, total_args);
  }

  co = dstrtol(argv[1], NULL, 10);
  if (mbox_send(co, sizeof(co), CO_MSG) != MBOX_SUCCESS) {
    LOG("o2 send failure");
    Exit();
  }

  Printf("CO molecule made, %d\n", getpid());
  semSignalOrDie(sem);
}
