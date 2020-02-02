#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"
#include "common.h"
#include "circular_buffer.h"
#include "utils.h"

void fillBuffer(ConsumerProducerContext ctxt, const char* const word) {
  int i;
  for (i = 0; i < dstrlen(word); ++i) {
    GUARD_EXPRESSIONS_CONS_PROD_CTXT(
        ctxt.producer_sem, ctxt.consumer_sem, ctxt.shared_mu,
        write(ctxt.circ_buffer_ptr, &word[i]),
        Printf("Producer %d inserted %c\n", getpid(), word[i]));
  }
}

void main(int argc, char** argv) {
  ConsumerProducerContext ctxt = decodeArgs(argc, argv);
  fillBuffer(ctxt, HELLO_WORLD_STR);
  cleanAndSignal(ctxt);
}
