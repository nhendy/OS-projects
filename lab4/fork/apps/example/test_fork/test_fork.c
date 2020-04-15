#include "misc.h"
#include "usertraps.h"

void main(int argc, char *argv[]) {
  int child_pid;
  int x = 200;

  Printf("Test : testing fork PID = %d \n", getpid());
  Printf("Checking x(%d) before fork PID = %d\n", x, getpid());
  child_pid = fork();
  // test if child
  if (child_pid == 0) {
    Printf("This is child the process with PID %d \n", getpid());
    Printf(
        "Before x changed: pid:%d checking x(%d) after fork child PID = %d\n",
        getpid(), x, child_pid);
    // change x to see if forked properly and
    x = 320;
    Printf("After x changed: pid:%d checking x(%d) after fork child PID = %d\n",
           getpid(), x, child_pid);
  } else {
    Printf("This is parent the process with pid %d \n", getpid());
    Printf(
        "Before x changed: pid:%d checking x(%d) after fork child PID = %d\n",
        getpid(), x, child_pid);
    // change x to see if forked properly and
    x = 430;
    Printf("After x changed: pid:%d checking x(%d) after fork child PID = %d\n",
           getpid(), x, child_pid);
  }

  Printf("Successfully Forked!\n");
}
