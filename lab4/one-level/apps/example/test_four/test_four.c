#include "usertraps.h"
#include "misc.h"

int fibonnaci(int x) {
  if (x <= 1) return x;
  return fibonnaci(x - 1) + fibonnaci(x - 2);
}

void main(int argc, char *argv[]) {
  int x = 9;
  Printf(
      "test 4 : running recursive fibonnaci to test increasing call stack\n");
  Printf("fibonnaci(%d) : %d\n", x, fibonnaci(x));
}
