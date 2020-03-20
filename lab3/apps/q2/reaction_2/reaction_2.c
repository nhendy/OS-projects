#include "utils.h"
#include "common.h"
#include "usertraps.h"
#include "misc.h"

void main(int argc, char *argv[]) {
  mbox_t co, o2, c2;
  sem_t sem;
  int total_args, k;
  int i;
  char message[sizeof(CO_MSG)];
  total_args = 5;
  if (argc < total_args) {
    LOG("Two few args in Reactions. Exiting...\n");
    Printf("Expected %d, got %d\n", 3, total_args);
  }
  // TODO: ask about "any loops" comment, i have loops but not for reactions"
  co = dstrtol(argv[1], NULL, 10);
  o2 = dstrtol(argv[2], NULL, 10);
  c2 = dstrtol(argv[3], NULL, 10);
  sem = dstrtol(argv[4], NULL, 10);

  for (k = 0; k < 4; k++) {
    if (mbox_recv(co, sizeof(co), (char *)message) != MBOX_SUCCESS) {
      LOG("CO receive failure");
      Exit();
    }
  }
  for (k = 0; k < 2; k++) {
    if (mbox_send(o2, sizeof(o2), O2_MSG) != MBOX_SUCCESS) {
      LOG("o2 send failure");
      Exit();
    }
  }
  for (k = 0; k < 2; k++) {
    if (mbox_send(c2, sizeof(c2), C2_MSG) != MBOX_SUCCESS) {
      LOG("c2 send failure");
      Exit();
    }
  }
  semSignalOrDie(sem);
}
