#ifndef _memory_h_
#define _memory_h_

// Put all your #define's in memory_constants.h
#include "memory_constants.h"

extern int lastosaddress;  // Defined in an assembly file

//--------------------------------------------------------
// Existing function prototypes:
//--------------------------------------------------------

int MemoryGetSize();
void MemoryModuleInit();
uint32 MemoryTranslateUserToSystem(PCB *pcb, uint32 addr);
int MemoryMoveBetweenSpaces(PCB *pcb, unsigned char *system,
                            unsigned char *user, int n, int dir);
int MemoryCopySystemToUser(PCB *pcb, unsigned char *from, unsigned char *to,
                           int n);
int MemoryCopyUserToSystem(PCB *pcb, unsigned char *from, unsigned char *to,
                           int n);
int MemoryPageFaultHandler(PCB *pcb);
void MemoryFreePte(uint32 pte);

static int negativeone = 0xFFFFFFFF;
static uint32 invert(uint32 n) { return (n ^ negativeone); }
//---------------------------------------------------------
// Put your function prototypes here
//---------------------------------------------------------
// All function prototypes including the malloc and mfree functions go here

void *malloc(PCB *pcb, int size);
void *mfree(PCB *pcb, void *ptr);
#endif  // _memory_h_
