#include "utils.h"
#include "common.h"
#include "usertraps.h"
#include "misc.h"

void main(int argc, char *argv[]) {

  mbox_t s2;
  sem_t sem;
  int total_args, k;
  const char kDummy = 'C';
  char message_s2[sizeof(S2_MSG)];
  total_args = 2;
  if (argc < total_args) {
    LOG("Too few args in Injector 1. Exiting...\n");
    Printf("Expected %d, got %d\n", total_args, argc);
  }

  s2 = dstrtol(argv[2], NULL, 10);
  if (mbox_send(s2, sizeof(s2), S2_MSG) = !MBOX_SUCCESS) {
    LOG("s2 send failure");
    Exit();
  }

  Printf("S2 molecule made, %d\n", getpid());

  semSignalOrDie(sem);
}
