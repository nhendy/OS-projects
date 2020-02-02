#ifndef __UTILS_HEADER__
#define __UTILS_HEADER__
#include "common.h"
ConsumerProducerContext decodeArgs(const int argc,
                                   const char* const* const argv);

void cleanAndSignal(const ConsumerProducerContext ctxt);
#endif
