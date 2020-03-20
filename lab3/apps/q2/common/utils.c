#include "common.h"
#include "utils.h"
#include "misc.h"
#include "usertraps.h"

void semSignalOrDie(sem_t sem) {
  if (sem_signal(sem) == SYNC_FAIL) {
    Printf("Weird. Couldn't sem_signal sem: %d, pid: %d\n", sem, getpid());
    Exit();
  }
}

void semWaitOrDie(sem_t sem) {
  if (sem_wait(sem) == SYNC_FAIL) {
    Printf("Weird. Couldn't sem_wait sem: %d, pid: %d\n", sem, getpid());
    Exit();
  }
}
