#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "synch.h"
#include "queue.h"
#include "mbox.h"

#define GUARDED_SCOPE(mu) \
  for (LockHandleAcquire(mu); false; LockHandleRelease(mu))

#define UNINTERRUPTIBLE_SCOPE(interrupts) \
  interrupts = DisableIntrs();            \
  for (; false; RestoreIntrs(interrupts))

#define LEN_OF_ARRAY(array) sizeof(array) / sizeof(array[0])

#define RETURN_FAIL_IF_FALSE(val) \
  if (!val) return MBOX_FAIL

#define RETURN_FAIL_IF_NULL(ptr) \
  if (ptr == NULL) return MBOX_FAIL

void bcopy(char* src, char* dst, int count) {
  while (count-- > 0) {
    *(dst++) = *(src++);
  }
}

int min(int a, int b) { return ((a < b) ? a : b); }

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
    mboxes[i].mbox_producers_sem = SemCreate(MBOX_MAX_BUFFERS_PER_MBOX);
    mboxes[i].mbox_consumers_sem = SemCreate(0);
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

mbox_t MboxCreate() {
  mbox_t mbox_handle;
  uint32 interrupts;
  UNINTERRUPTIBLE_SCOPE(interrupts) {
    for (mbox_handle = 0; mbox_handle < MBOX_NUM_MBOXES; ++mbox_handle) {
      if (mboxes[mbox_handle].inuse == 0) break;
    }
  }
  if (mbox_handle == MBOX_NUM_MBOXES) return MBOX_FAIL;
  if (MboxInit(&mboxes[mbox_handle]) == MBOX_FAIL) return MBOX_FAIL;
  return mbox_handle;
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
//-------------------------------------------------------
int MboxOpenInternal(Mbox* mbox) {
  Link* l;
  uint32 interrupts;
  UNINTERRUPTIBLE_SCOPE(interrupts) {
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
int MboxCloseInternal(Mbox* mbox, int pid) {
  uint32 interrupts;
  Link* l;
  PCB* pcb;
  UNINTERRUPTIBLE_SCOPE(interrupts) {
    // TODO:(nhendy) look up by pid
    if (AQueueEmpty(&mbox->pids)) {
      // Need to restore interrupts manually
      // because of premature exit.
      RestoreIntrs(interrupts);
      return MBOX_SUCCESS;
    }
    l = AQueueFirst(&mbox->pids);
    while (l) {
      pcb = (PCB*)AQueueObject(l);
      if (GetPidFromAddress(pcb) == pid) break;
    }
    // If we reach the end node that means the
    // pid doesn't exist.
    if (l == NULL) {
      // Need to restore interrupts manually
      // because of premature exit.
      RestoreIntrs(interrupts);
      return MBOX_FAIL;
    }

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

int MboxClose(mbox_t handle) {
  if (!SanityCheckHandle(handle)) return MBOX_FAIL;
  return MboxCloseInternal(&mboxes[handle], GetCurrentPid());
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
int MboxMessageInit(MboxMessage* mssg, int length, void* message) {
  GUARDED_SCOPE(mssg->mssg_lock) {
    bcopy(message, mssg->message, length);
    mssg->length = length;
  }
  return MBOX_SUCCESS;
}

int MboxMessageCreate(int length, void* message) {
  int i;
  mbox_mssg_t mssg_handle;
  uint32 interrupts;
  if (length > MBOX_MAX_MESSAGE_LENGTH) return MBOX_FAIL;
  UNINTERRUPTIBLE_SCOPE(interrupts) {
    for (mssg_handle = 0; mssg_handle < LEN_OF_ARRAY(mboxes_messages);
         ++mssg_handle) {
      if (mboxes_messages[i].inuse == 0) break;
    }
  }
  if (mssg_handle == LEN_OF_ARRAY(mboxes_messages)) return MBOX_FAIL;
  if (MboxMessageInit(&mboxes_messages[mssg_handle], length, message) ==
      MBOX_FAIL)
    return MBOX_FAIL;
  return MBOX_SUCCESS;
}

int MboxSendInternal(Mbox* mbox, MboxMessage* mssg) {
  Link* l;
  uint32 interrupts;
  // This needs to be mutually exclusive
  // because AQueueAlloc allocates a link
  // from a global freeLinks array which
  // is no thread-aware. It's not locked
  // because it's protecting a system level
  // non-mbox related resouce, rather it's
  // protected from interrupts.
  UNINTERRUPTIBLE_SCOPE(interrupts) {
    if ((l = AQueueAllocLink((void*)mssg)) == NULL) {
      printf(
          "FATAL ERROR: could not allocate link for mssg queue in "
          "MboxSendInternal!\n");
      exitsim();
    }
  }
  // SemWait here in order to
  // prevent allocating more mssgs than the maximum
  // allowable mssgs in a mbox.
  SemHandleWait(mbox->mbox_producers_sem);
  // Guard write/read access to messages queue
  // since it's mbox shared resource.
  GUARDED_SCOPE(mbox->mbox_lock) {
    if (AQueueInsertLast(&mbox->messages, l) != QUEUE_SUCCESS) {
      printf(
          "FATAL ERROR: could not insert new link into pids  queue "
          "in MboxOpen!\n");
      exitsim();
    }
  }
  SemHandleSignal(mbox->mbox_consumers_sem);
  return MBOX_SUCCESS;
}
int MboxOpenedByPid(Mbox* mbox) {
  Link* l;
  PCB* pcb;
  uint32 interrupts;
  UNINTERRUPTIBLE_SCOPE(interrupts) {
    l = AQueueFirst(&mbox->pids);
    while (l) {
      pcb = (PCB*)AQueueObject(l);
      if (GetCurrentPid() == GetPidFromAddress(pcb)) {
        // We need to restore interrupts manually
        // because this is exiting the scopre
        // prematurely.
        RestoreIntrs(interrupts);
        return true;
      }
    }
  }
  return false;
}

int MboxSend(mbox_t handle, int length, void* message) {
  mbox_mssg_t mssg_handle;
  if (!SanityCheckHandle(handle)) return MBOX_FAIL;
  if (!MboxOpenedByPid(&mboxes[handle])) return MBOX_FAIL;
  if ((mssg_handle = MboxMessageCreate(length, message)) == MBOX_FAIL)
    return MBOX_FAIL;
  return MboxSendInternal(&mboxes[handle], &mboxes_messages[mssg_handle]);
}
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
int SanityCheckRequestedLength(MboxMessage* mssg, int maxlength) {
  int result = true;
  GUARDED_SCOPE(mssg->mssg_lock) { result = mssg->length <= maxlength; }
  return result;
}

int ReadOutMessageData(MboxMessage* mssg, int maxlength, void* message) {
  GUARDED_SCOPE(mssg->mssg_lock) {
    bcopy(mssg->message, message, min(maxlength, mssg->length));
    mssg->inuse = 0;
    mssg->length = 0;
  }
  return MBOX_SUCCESS;
}

int MboxRecvInternal(Mbox* mbox, int maxlength, void* message) {
  Link* l;
  uint32 interrupts;
  MboxMessage* mssg;
  RETURN_FAIL_IF_NULL(mssg);
  GUARDED_SCOPE(mbox->mbox_lock) {
    l = AQueueFirst(&mbox->messages);
    mssg = (MboxMessage*)AQueueObject(l);
  }

  RETURN_FAIL_IF_FALSE(SanityCheckRequestedLength(mssg, maxlength));

  SemHandleWait(mbox->mbox_consumers_sem);
  GUARDED_SCOPE(mbox->mbox_lock) {
    UNINTERRUPTIBLE_SCOPE(interrupts) {
      if (AQueueRemove(&l) != QUEUE_SUCCESS) {
        printf(
            "FATAL ERROR: could not remove link from messages queue in "
            "MboxRecvInternal!\n");
        exitsim();
      }
    }
  }
  SemHandleSignal(mbox->mbox_producers_sem);

  return ReadOutMessageData(mssg, maxlength, message);
}

int MboxRecv(mbox_t handle, int maxlength, void* message) {
  if (!SanityCheckHandle(handle)) return MBOX_FAIL;
  if (!MboxOpenedByPid(&mboxes[handle])) return MBOX_FAIL;
  return MboxRecvInternal(&mboxes[handle], maxlength, message);
}

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
int MboxCloseAllByPid(int pid) {
  mbox_t handle;
  for (handle = 0; handle < LEN_OF_ARRAY(mboxes); ++handle) {
    if (MboxCloseInternal(&mboxes[handle], pid) == MBOX_SUCCESS) {
      dbprintf('y', "Successfully removed PID %d from Mbox %d", pid, handle);
    } else {
      dbprintf('y', "PID %d not in Mbox %d", pid, handle);
    }
  }
  return MBOX_SUCCESS;
}
