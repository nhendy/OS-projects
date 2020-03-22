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
    LOG("Two few args in Reaction 2. Exiting...\n");
    Printf("Expected %d, got %d\n", 3, total_args);
  }
  // TODO: ask about "any loops" comment, i have loops but not for reactions"
  sem = dstrtol(argv[1], NULL, 10);
  co = dstrtol(argv[2], NULL, 10);
  o2 = dstrtol(argv[3], NULL, 10);
  c2 = dstrtol(argv[4], NULL, 10);

  if (mbox_open(co) == MBOX_FAIL) {
    LOG("Error in opening co mbox\n");
    Exit();
  }
  if (mbox_open(c2) == MBOX_FAIL) {
    LOG("Error in opening c2 mbox\n");
    Exit();
  }
  if (mbox_open(o2) == MBOX_FAIL) {
    LOG("Error in opening o2 mbox\n");
    Exit();
  }
  Printf("reaction_2(%d): Co mbox: %d\n", getpid(), co);
  for (k = 0; k < 4; k++) {
    if (mbox_recv(co, sizeof(CO_MSG), (void *)message) == MBOX_FAIL) {
      LOG("CO receive failure\n");
      Exit();
    }
    Printf("reaction_2 (%d): Received CO\n", getpid());
  }
  for (k = 0; k < 2; k++) {
    if (mbox_send(o2, sizeof(O2_MSG), O2_MSG) != MBOX_SUCCESS) {
      LOG("o2 send failure\n");
      Exit();
    }
    Printf("o2 molecule made\n");
  }

  for (k = 0; k < 2; k++) {
    if (mbox_send(c2, sizeof(C2_MSG), C2_MSG) != MBOX_SUCCESS) {
      LOG("c2 send failure\n");
      Exit();
    }
    Printf("c2 molecule made\n");
  }
  Printf("Reaction_2 (%d): Good bye!!!!\n", getpid());
  semSignalOrDie(sem);
}
