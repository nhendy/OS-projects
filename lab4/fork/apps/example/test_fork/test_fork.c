#include "misc.h"
#include "usertraps.h"

void main(int argc, char *argv[]) {
  int child_pid;
  int child_pid2;
  int x = 200;

  Printf("Test : testing fork PID = %d \n", getpid());
  Printf("Checking x(%d) before fork PID = %d\n", x, getpid());
  child_pid = fork();
  child_pid2 = fork();
  // test if child

  if (child_pid2 == 0) {
    Printf("This is child 2 the process with PID %d \n", getpid());
    Printf(
        "Before x changed: pid:%d checking x(%d) after fork child 2 child_pid "
        "= %d\n",
        getpid(), x, child_pid2);
    // change x to see if forked properly and
    x = 7000;
    Printf(
        "After x changed: pid:%d checking x(%d) after fork child 2 PID = %d\n",
        getpid(), x, child_pid2);
  }  // test if child

  if (child_pid == 0) {
    Printf("This is child the process with PID %d \n", getpid());
    Printf(
        "Before x changed: pid:%d checking x(%d) after fork child PID = %d\n",
        getpid(), x, child_pid);
    // change x to see if forked properly and
    x = 320;
    Printf("After x changed: pid:%d checking x(%d) after fork child PID = %d\n",
           getpid(), x, child_pid);
  }
  Printf("Successfully Forked!\n");
}
