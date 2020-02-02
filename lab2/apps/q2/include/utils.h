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
#endif
