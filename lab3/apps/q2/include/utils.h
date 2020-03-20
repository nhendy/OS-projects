// #ifndef __UTILS_HEADER_Q2__
// #define __UTILS_HEADER_Q2__
#include "common.h"
#include "usertraps.h"
void semSignalOrDie(sem_t sem);
// // Same as above for waiting a semaphore.
// void semWaitOrDie(sem_t sem);
#define LOG(msg)              \
  Printf("[%d]: ", __LINE__); \
  Printf(msg);
// #endif
