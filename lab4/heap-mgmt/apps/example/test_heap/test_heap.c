#include "usertraps.h"
#include "misc.h"

void main(int argc, char *argv[]) {
  int *ptr;
  Printf("test_heap malloc'ing %d (%d): \n", sizeof(int), getpid());
  ptr = malloc(sizeof(int));
  Printf("freeing %d bytes \n", mfree(ptr));
  ptr = malloc(sizeof(int));
  Printf("freeing %d bytes \n", mfree(ptr));
  ptr = malloc(sizeof(int));
  Printf("freeing %d bytes \n", mfree(ptr));
  ptr = malloc(sizeof(int) * 10);
  Printf("freeing %d bytes \n", mfree(ptr));
  ptr = malloc(1 << 12);
  Printf("freeing %d bytes \n", mfree(ptr));
}
