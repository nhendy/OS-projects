#include "usertraps.h"
#include "misc.h"

int main(int argc, char **argv) {
  mbox_t h_mbox;  // Used to hold handle to mailbox
  char recvd;
  const char kDummy = 'C';
  const mbox_t kBadHandle = -999;
  if ((h_mbox = mbox_create()) == MBOX_FAIL) {
    Printf("test (%d): ERROR: could not allocate mailbox!", getpid());
    Exit();
  }
  // First time should succeed
  if (mbox_open(h_mbox) == MBOX_FAIL) {
    Printf("makeprocs (%d): Could not open mailbox %d!\n", getpid(), h_mbox);
    Exit();
  }
  // Second open should fail.
  if (mbox_open(h_mbox) != MBOX_FAIL) {
    Printf("makeprocs (%d): Could not open mailbox %d!\n", getpid(), h_mbox);
    Exit();
  }
  // Should fail due to bad handle
  if (mbox_send(kBadHandle, sizeof(int), (void *)&kDummy) != MBOX_FAIL) {
    Printf("Should fail due to bad handle but didnt get MBOX_FAIL");
    Exit();
  }
  // Should succeed
  if (mbox_send(h_mbox, sizeof(char), (void *)&kDummy) == MBOX_FAIL) {
    Printf("Should succeed but got MBOX_FAIL\n");
    Exit();
  }
  // Should fail to recv due to bad handle
  if (mbox_recv(kBadHandle, sizeof(char), (void *)&kDummy) != MBOX_FAIL) {
    Printf("Should fail to recv due to bad handle\n");
    Exit();
  }
  // Should succeed
  if (mbox_recv(h_mbox, sizeof(char), (void *)&recvd) == MBOX_FAIL) {
    Printf("Should succeed to recv but failed\n");
    Exit();
  }
  if (recvd != kDummy) {
    Printf("Didnt recevie the right char %c\n", recvd);
    Exit();
  }
  // Bad handle
  if (mbox_close(kBadHandle) != MBOX_FAIL) {
    Printf("Should fail to close due to bad handle\n");
    Exit();
  }
  // Should succeed
  if (mbox_close(h_mbox) == MBOX_FAIL) {
    Printf("Should succeed to close\n");
    Exit();
  }
}
