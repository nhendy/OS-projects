#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"
#include "common.h"
#include "circular_buffer.h"
#include "utils.h"

void removeFromBuffer(ConsumerProducerContext ctxt, const char* const word) {
  int i;
  char result;
  for (i = 0; i < dstrlen(word); ++i) {
    semWaitOrDie(ctxt.consumer_sem);
    lockAcquireOrDie(ctxt.shared_mu);
    read(ctxt.circ_buffer_ptr, &result);
    Printf("Consumer %d removed %c\n", getpid(), result);
    lockReleaseOrDie(ctxt.shared_mu);
    semSignalOrDie(ctxt.producer_sem);
  }
}

void main(int argc, char** argv) {
  ConsumerProducerContext ctxt = decodeArgs(argc, argv);
  removeFromBuffer(ctxt, HELLO_WORLD_STR);
  cleanAndSignal(ctxt);
}
