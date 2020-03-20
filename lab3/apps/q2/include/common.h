#ifndef __COMMON_HEADER_Q5__
#define __COMMON_HEADER_Q5__
// #include "lab2-api.h"

// Message strings of all mollcules used for send and recieve
#define S2_MSG "S2"
#define S_MSG "S"
#define CO_MSG "CO"
#define O2_MSG "O2"
#define C2_MSG "C2"
#define SO4_MSG "SO4"

// Macros for test truthy/falsey return values
#define FALSE 0
#define TRUE 1
// Maros for invalid semaphore values and amount_needed
// variables. Useful as sentinels that an object
// wasn't fully constructed yet.
#define INVALID_SEMAPHORE_CONST -1
#define INVALID_AMOUNT_NEEDED_CONST -1
// Maximum number of molecules allowed.
#define NUM_MOLECULES 100
// Names of children processes.
#define INJECTOR_BINARY "injector.dlx.obj"
#define REACTION_BINARY "reaction.dlx.obj"

#endif
