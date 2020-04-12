#include "usertraps.h"
#include "misc.h"
#include "os/memory_constants.h"

void main(int argc, char *argv[]) {
  int *max_virtual_address = MAX_VIRTUAL_ADDRESS;
  int *unalloc_address =
      (MAX_VIRTUAL_ADDRESS + 1 - 2 * MEM_PAGE_SIZE) & 0xfffffffc;

  Printf(
      "Test 3 : Accessing unallocated address %d while max virtual address is "
      "%d\n",
      unalloc_address, max_virtual_address);
  Printf("%d\n", *(unalloc_address));
}
