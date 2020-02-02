#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"
#include "common.h"
#include "circular_buffer.h"
#include "utils.h"

void fillBuffer(ConsumerProducerContext ctxt, const char* const word) {
  int i;
  for (i = 0; i < dstrlen(word); ++i) {
    semWaitOrDie(ctxt.producer_sem);
    lockAcquireOrDie(ctxt.shared_mu);
    write(ctxt.circ_buffer_ptr, &word[i]);
    Printf("Producer %d inserted %c\n", getpid(), word[i]);
    lockReleaseOrDie(ctxt.shared_mu);
    semSignalOrDie(ctxt.consumer_sem);
  }
}

void main(int argc, char** argv) {
  ConsumerProducerContext ctxt = decodeArgs(argc, argv);
  fillBuffer(ctxt, HELLO_WORLD_STR);
  cleanAndSignal(ctxt);
}
