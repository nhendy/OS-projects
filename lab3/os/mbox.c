#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "synch.h"
#include "queue.h"
#include "mbox.h"

#define UNINTERRUPTIBLE_SCOPE(interrupts) \
  interrupts = DisableIntrs();            \
  for (; false; RestoreIntrs(interrupts))

#define LEN_OF_ARRAY(array) sizeof(array) / sizeof(array[0])

int SanityCheckHandle(mbox_t handle) {
  if (handle < 0) return false;
  if (handle >= MBOX_NUM_MBOXES) return false;
  if (!mboxes[handle].inuse) return false;
  return true;
}

//-------------------------------------------------------
//
// void MboxModuleInit();
//
// Initialize all mailboxes.  This process does not need
// to worry about synchronization as it is called at boot
// time.  Only initialize necessary items here: you can
// initialize others in MboxCreate.  In other words,
// don't waste system resources like locks and semaphores
// on unused mailboxes.
//
//-------------------------------------------------------

void MboxModuleInit() {
  int i;
  for (i = 0; i < LEN_OF_ARRAY(mboxes); ++i) {
    mboxes[i].inuse = 0;
    mboxes[i].mbox_lock = LockCreate();
  }
  for (i = 0; i < LEN_OF_ARRAY(mboxes_messages); ++i) {
    mboxes_messages[i].inuse = 0;
    mboxes_messages[i].mssg_lock = LockCreate();
  }
}
//-------------------------------------------------------
//
// mbox_t MboxCreate();
//
// Allocate an available mailbox structure for use.
//
// Returns the mailbox handle on success
// Returns MBOX_FAIL on error.
//
//-------------------------------------------------------
mbox_t MboxCreate() {
  mboxt_t;
  uint32 interrupts;
  UNINTERRUPTIBLE_SCOPE(interrupts) {
    for (mbox_t = 0; mbox_t < MBOX_NUM_MBOXES; ++mboxt_t) {
      // Acquire lock before inspecting `inuse` flag
      // LockHandleAcquire(mboxes[mbox_t].mbox_lock);
      if (mboxes[mbox_t].inuse == 0) break;
      // if it's inuse release lock. If not the lock
      // will stay acquired by the current process and release
      // after MboxInit
      // LockHandleRelease(mboxes[mboxes_t].mbox_lock);
    }
    if (mbox_t == MBOX_NUM_MBOXES) return MBOX_FAIL;
    if (MboxInit(&mboxes[mbox_t]) == MBOX_FAIL) return MBOX_FAIL;
    // LockHandlRelease(mboxes[mboxes_t].mbox_lock);
  }
  return mbox_t;
}

int MboxInit(Mbox* mbox) {
  if (!mbox) return MBOX_FAIL;
  if (AQueueInit(&mbox->messages) != QUEUE_SUCCESS) {
    printf(
        "FATAL ERROR: could not initialize mbox messages queue in "
        "MboxInit!\n");
    exitsim();
  }
  if (AQueueInit(&mbox->pids) != QUEUE_SUCCESS) {
    printf(
        "FATAL ERROR: could not initialize mbox pids queue in "
        "MboxInit!\n");
    exitsim();
  }
  mbox->inuse = 1;
  return MBOX_SUCCESS;
}
//-------------------------------------------------------
//
// void MboxOpen(mbox_t);
//
// Open the mailbox for use by the current process.  Note
// that it is assumed that the internal lock/mutex handle
// of the mailbox and the inuse flag will not be changed
// during execution.  This allows us to get the a valid
// lock handle without a need for synchronization.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
// TODO: (nhendy) clarify what does it mean to not change inuse? semantics of
// inuse?
//-------------------------------------------------------
int MboxOpenInternal(Mbox* mbox) {
  Link* l;
  uint32 interrupts;
  UNINTERRUPTIBLE_SCOPE(interrupts) {
    if (mbox->inuse == 0) {
      return MBOX_FAIL;
    }
    if ((l = AQueueAllocLink((void*)currentPCB)) == NULL) {
      printf(
          "FATAL ERROR: could not allocate link for pids queue in "
          "MboxOpen!\n");
      exitsim();
    }
    if (AQueueInsertLast(&mbox->pids, l) != QUEUE_SUCCESS) {
      printf(
          "FATAL ERROR: could not insert new link into pids  queue "
          "in MboxOpen!\n");
      exitsim();
    }
  }
  return MBOX_SUCCESS;
}

int MboxOpen(mbox_t handle) {
  if (!SanityCheckHandle(handle)) return MBOX_FAIL;
  return MboxOpenInternal(&mboxes[handle]);
}

//-------------------------------------------------------
//
// int MboxClose(mbox_t);
//
// Close the mailbox for use to the current process.
// If the number of processes using the given mailbox
// is zero, then disable the mailbox structure and
// return it to the set of available mboxes.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//-------------------------------------------------------
int MboxClose(mbox_t handle) {
  if (!SanityCheckHandle(handle)) return MBOX_FAIL;
  return MboxCloseInternal(&mboxes[handles]);
}

int MboxCloseInternal(Mbox* mbox) {
  uint32 interrupts;
  Link* l;
  PCB* pcb;
  UNINTERRUPTIBLE_SCOPE(interrupts) {
    if (AQueueEmpty(&mbox->pids)) return MBOX_SUCCESS;
    l = AQueueFirst(&mbox->pids);
    pcb = (PCB*)AQueueObject(l);

    if (AQueueRemove(&l) != QUEUE_SUCCESS) {
      printf(
          "FATAL ERROR: could not remove link from pids queue in "
          "MboxCloseInternal!\n");
      exitsim();
    }
    if (AQueueEmpty(&mbox->pids)) mbox->inuse = 0;
  }
  return MBOX_SUCCESS;
}

//-------------------------------------------------------
//
// int MboxSend(mbox_t handle,int length, void* message);
//
// Send a message (pointed to by "message") of length
// "length" bytes to the specified mailbox.  Messages of
// length 0 are allowed.  The call
// blocks when there is not enough space in the mailbox.
// Messages cannot be longer than MBOX_MAX_MESSAGE_LENGTH.
// Note that the calling process must have opened the
// mailbox via MboxOpen.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//-------------------------------------------------------
int MboxSend(mbox_t handle, int length, void* message) { return MBOX_FAIL; }

//-------------------------------------------------------
//
// int MboxRecv(mbox_t handle, int maxlength, void* message);
//
// Receive a message from the specified mailbox.  The call
// blocks when there is no message in the buffer.  Maxlength
// should indicate the maximum number of bytes that can be
// copied from the buffer into the address of "message".
// An error occurs if the message is larger than maxlength.
// Note that the calling process must have opened the mailbox
// via MboxOpen.
//
// Returns MBOX_FAIL on failure.
// Returns number of bytes written into message on success.
//
//-------------------------------------------------------
int MboxRecv(mbox_t handle, int maxlength, void* message) { return MBOX_FAIL; }

//--------------------------------------------------------------------------------
//
// int MboxCloseAllByPid(int pid);
//
// Scans through all mailboxes and removes this pid from their "open procs"
// list.
// If this was the only open process, then it makes the mailbox available.  Call
// this function in ProcessFreeResources in process.c.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//--------------------------------------------------------------------------------
int MboxCloseAllByPid(int pid) { return MBOX_FAIL; }
