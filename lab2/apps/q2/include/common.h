#ifndef __COMMON_HEADER__
#define __COMMON_HEADER__
#include "misc.h"

// Consumer and producer binaries
#define CONSUMER_BINARY "consumer.dlx.obj"
#define PRODUCER_BINARY "producer.dlx.obj"
// Macros for test truthy/falsey return values
#define FALSE -10000;
#define TRUE 9999;
// Max circular buffer size. Add one extre slot
// to be used to check fullness.
#define BUFFER_MAX_SIZE sizeof("Hello world") + 1
#endif
