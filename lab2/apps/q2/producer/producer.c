#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"
#include "common.h"
#include "circular_buffer.h"
#include "utils.h"

void main(int argc, char** argv) {
  ConsumerProducerContext ctxt = decodeArgs(argc, argv);
  cleanAndSignal(ctxt);
}
