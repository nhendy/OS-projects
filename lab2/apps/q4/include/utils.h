#ifndef __UTILS_HEADER__
#define __UTILS_HEADER__
#include "common.h"
// Decodes command line args passed from makeprocs. Used in producer
// and consumer to unify decoding routine.
ConsumerProducerContext decodeArgs(const int argc,
                                   const char* const* const argv);
// Clean up routine called at the end of the producer and consumer. Used
// to signal the all_proccess_done semaphore so that makeprocs process exits.
void cleanAndSignal(const ConsumerProducerContext ctxt);
// Acquire lock or exit process if invalid.
void lockAcquireOrDie(lock_t mu);
// Same as above for releasing a lock.
void lockReleaseOrDie(lock_t mu);
// Same as above for signaling a semaphore.
void semSignalOrDie(sem_t sem);
// Same as above for waiting a semaphore.
void semWaitOrDie(sem_t sem);
// Same as above for signaling a  cond.
void condSignalOrDie(cond_t cond);
// Same as above for waiting a cond.
void condWaitOrDie(cond_t cond);
#endif
