#include "utils.h"
#include "common.h"
#include "usertraps.h"
#include "misc.h"

void main(int argc, char *argv[]) {

  mbox_t co;
  sem_t sem;
  int total_args, k;
  char message_s2[sizeof(CO_MSG)];
  total_args = 2;
  if (argc < total_args) {
    LOG("Too few args in Injector 2 . Exiting...\n");
    Printf("Expected %d, got %d\n", total_args, argc);
  }
  sem = dstrtol(argv[1], NULL, 10);
  co = dstrtol(argv[2], NULL, 10);
  if (mbox_open(co) == MBOX_FAIL) {
    LOG("Error in opening co mbox");
    Exit();
  }
  Printf("Co mbox: %d\n", co);
  if (mbox_send(co, sizeof(CO_MSG), CO_MSG) != MBOX_SUCCESS) {
    LOG("co send failure");
    Exit();
  }

  Printf("CO molecule made, %d\n", getpid());
  semSignalOrDie(sem);
}
