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
// TODO: (nhendy) looks like gcc-dlx doesn't support variadic macros.
#define GUARD_EXPRESSIONS_CONS_PROD_CTXT(sem_to_wait, sem_to_signal, mu, \
                                         expr1, expr2)                   \
  do {                                                                   \
    semWaitOrDie(sem_to_wait);                                           \
    lockAcquireOrDie(mu);                                                \
    expr1;                                                               \
    expr2;                                                               \
    lockReleaseOrDie(mu);                                                \
    semSignalOrDie(sem_to_signal);                                       \
  } while (0)

#endif
