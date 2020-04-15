#include "usertraps.h"
#include "misc.h"
#include "os/memory_constants.h"

void main(int argc, char *argv[]) {
  int *max_virtual_address = MAX_VIRTUAL_ADDRESS;

  Printf("Test 2 : Accessing address %d while max virtual address is %d\n",
         max_virtual_address + 1, max_virtual_address);
  Printf("%d\n", *(max_virtual_address + 1));
}
