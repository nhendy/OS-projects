#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "synch.h"
#include "queue.h"
#include "mbox.h"

#define GUARDED_SCOPE(mu, dummy) \
  dummy = true;                  \
  for (LockHandleAcquire(mu); dummy; LockHandleRelease(mu), dummy = false)

#define UNINTERRUPTIBLE_SCOPE(interrupts, dummy) \
  dummy = true;                                  \
  interrupts = DisableIntrs();                   \
  for (; dummy; RestoreIntrs(interrupts), dummy = false)

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

int MboxOpenedByPid(Mbox* mbox) {
  Link* l;
  PCB* pcb;
  uint32 interrupts;
  int dummy;
  dbprintf('y', "MboxOpenedByPid: Entering, PID: %d\n", GetCurrentPid());
  UNINTERRUPTIBLE_SCOPE(interrupts, dummy) {
    l = AQueueFirst(&mbox->pids);
    while (l) {
      pcb = (PCB*)AQueueObject(l);
      if (GetCurrentPid() == GetPidFromAddress(pcb)) {
        // We need to restore interrupts manually
        // because this is exiting the scopre
        // prematurely.
        RestoreIntrs(interrupts);
        dbprintf('y', "MboxOpenedByPid: Exiting, PID: %d\n", GetCurrentPid());
        return true;
      }
      l = AQueueNext(l);
    }
  }
  dbprintf('y', "MboxOpenedByPid: Exiting, PID: %d\n", GetCurrentPid());
  return false;
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
  dbprintf('y', "MboxModuleInit: %d mboxes,%d mssgs\n", LEN_OF_ARRAY(mboxes),
           LEN_OF_ARRAY(mboxes_messages));
  for (i = 0; i < LEN_OF_ARRAY(mboxes); ++i) {
    mboxes[i].inuse = 0;
  }
  for (i = 0; i < LEN_OF_ARRAY(mboxes_messages); ++i) {
    mboxes_messages[i].inuse = 0;
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
  RETURN_FAIL_IF_NULL(mbox);
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
  mbox->mbox_lock = LockCreate();
  mbox->mbox_producers_sem = SemCreate(MBOX_MAX_BUFFERS_PER_MBOX);
  mbox->mbox_consumers_sem = SemCreate(0);
  return MBOX_SUCCESS;
}

mbox_t MboxCreate() {
  mbox_t mbox_handle;
  uint32 interrupts;
  int dummy;
  dbprintf('y', "MboxCreate: Entering , PID: %d\n", GetCurrentPid());
  UNINTERRUPTIBLE_SCOPE(interrupts, dummy) {
    for (mbox_handle = 0; mbox_handle < LEN_OF_ARRAY(mboxes); ++mbox_handle) {
      if (mboxes[mbox_handle].inuse == 0) {
        dbprintf('y', "Handle %d is available\n", mbox_handle);
        break;
      }
    }
  }
  if (mbox_handle == LEN_OF_ARRAY(mboxes)) return MBOX_FAIL;
  if (MboxInit(&mboxes[mbox_handle]) == MBOX_FAIL) return MBOX_FAIL;
  dbprintf('y', "MboxCreate: Returning handle %d, PID: %d\n", mbox_handle,
           GetCurrentPid());
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
  int dummy;
  dbprintf('y', "MboxOpenInternal %d, PID: %d\n", (int)(mbox - mboxes),
           GetCurrentPid());
  UNINTERRUPTIBLE_SCOPE(interrupts, dummy) {
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
  dbprintf('y', "MboxOpenInternal: Done, PID: %d\n", GetCurrentPid());
  return MBOX_SUCCESS;
}

int MboxOpen(mbox_t handle) {
  dbprintf('y', "MboxOpen: Entering, PID: %d. Opening mbox: %d\n",
           GetCurrentPid(), handle);
  // Fail if the handle is corrupt
  if (!SanityCheckHandle(handle)) {
    printf("PID: %d. Failed to open mbox due to bad handle\n", GetCurrentPid());
    return MBOX_FAIL;
  }
  // Fail if it's already open
  if (MboxOpenedByPid(&mboxes[handle])) {
    printf("PID: %d. Failed to open mbox as it already opened it previously\n",
           GetCurrentPid());
    return MBOX_FAIL;
  }
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
  int dummy;
  UNINTERRUPTIBLE_SCOPE(interrupts, dummy) {
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
      l = AQueueNext(l);
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
  // Fail if the handle is corrupt
  if (!SanityCheckHandle(handle)) return MBOX_FAIL;
  // Fail if it's already open
  if (!MboxOpenedByPid(&mboxes[handle])) return MBOX_FAIL;
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
  int dummy;
  RETURN_FAIL_IF_NULL(mssg);
  mssg->mssg_lock = LockCreate();
  GUARDED_SCOPE(mssg->mssg_lock, dummy) {
    bcopy(message, mssg->message, length);
    mssg->length = length;
    mssg->inuse = 1;
  }
  dbprintf('y', "MboxMessageInit: Done length %d mssg %d, PID: %d!\n",
           mssg->length, (int)(mssg - mboxes_messages), GetCurrentPid());
  dbprintf('y', "MboxMessageInit: First byte: %c, PID: %d\n", mssg->message[1],
           GetCurrentPid());
  return MBOX_SUCCESS;
}

int MboxMessageCreate(int length, void* message) {
  int dummy;
  mbox_mssg_t mssg_handle;
  uint32 interrupts;
  if (length > MBOX_MAX_MESSAGE_LENGTH) return MBOX_FAIL;
  dbprintf('y', "MboxMessageCreate: Entering, PID: %d\n", GetCurrentPid());
  UNINTERRUPTIBLE_SCOPE(interrupts, dummy) {
    for (mssg_handle = 0; mssg_handle < LEN_OF_ARRAY(mboxes_messages);
         ++mssg_handle) {
      if (mboxes_messages[mssg_handle].inuse == 0) break;
    }
  }
  dbprintf('y', "MboxMessageCreate: Found %d, PID: %d\n", mssg_handle,
           GetCurrentPid());
  if (mssg_handle == LEN_OF_ARRAY(mboxes_messages)) return MBOX_FAIL;
  if (MboxMessageInit(&mboxes_messages[mssg_handle], length, message) ==
      MBOX_FAIL)
    return MBOX_FAIL;
  return mssg_handle;
}

int MboxSendInternal(Mbox* mbox, MboxMessage* mssg) {
  Link* l;
  uint32 interrupts;
  int dummy;
  // This needs to be mutually exclusive
  // because AQueueAlloc allocates a link
  // from a global freeLinks array which
  // is no thread-aware. It's not locked
  // because it's protecting a system level
  // non-mbox related resouce, rather it's
  // protected from interrupts.
  dbprintf('y', "MboxSendInternal: Entering, PID: %d\n", GetCurrentPid());
  UNINTERRUPTIBLE_SCOPE(interrupts, dummy) {
    if ((l = AQueueAllocLink((void*)mssg)) == NULL) {
      printf(
          "FATAL ERROR: could not allocate link for mssg queue in "
          "MboxSendInternal!\n");
      exitsim();
    }
  }
  dbprintf('y', "MboxSendInternal: Allocated Link, PID: %d\n", GetCurrentPid());
  // SemWait here in order to
  // prevent allocating more mssgs than the maximum
  // allowable mssgs in a mbox.
  SemHandleWait(mbox->mbox_producers_sem);
  // Guard write/read access to messages queue
  // since it's mbox shared resource.
  dbprintf('y', "MboxSendInternal: Before inserting in queue, PID: %d\n",
           GetCurrentPid());
  GUARDED_SCOPE(mbox->mbox_lock, dummy) {
    if (AQueueInsertLast(&mbox->messages, l) != QUEUE_SUCCESS) {
      printf(
          "FATAL ERROR: could not insert new link into pids  queue "
          "in MboxSendInternal!\n");
      exitsim();
    }
  }
  SemHandleSignal(mbox->mbox_consumers_sem);
  dbprintf('y', "MboxSendInternal: Done, inserted message %d, PID %d!\n",
           (int)(mssg - mboxes_messages), GetCurrentPid());
  return MBOX_SUCCESS;
}

int MboxSend(mbox_t handle, int length, void* message) {
  mbox_mssg_t mssg_handle;
  dbprintf('y', "MboxSend: Entering, PID: %d\n", GetCurrentPid());
  if (!SanityCheckHandle(handle)) return MBOX_FAIL;
  if (!MboxOpenedByPid(&mboxes[handle])) return MBOX_FAIL;
  if ((mssg_handle = MboxMessageCreate(length, message)) == MBOX_FAIL)
    return MBOX_FAIL;
  dbprintf('y', "MboxSend:Message %d, PID: %d\n", mssg_handle, GetCurrentPid());
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
  int result = true, dummy;
  GUARDED_SCOPE(mssg->mssg_lock, dummy) { result = mssg->length <= maxlength; }
  return result;
}

int ReadOutMessageData(MboxMessage* mssg, int maxlength, void* message) {
  int dummy;
  int num_bytes = min(maxlength, mssg->length);
  RETURN_FAIL_IF_NULL(mssg);
  RETURN_FAIL_IF_NULL(message);
  dbprintf('y', "ReadOutMessageData: length %d, mssg %d, PID: %d\n", num_bytes,
           (int)(mssg - mboxes_messages), GetCurrentPid());
  GUARDED_SCOPE(mssg->mssg_lock, dummy) {
    bcopy(mssg->message, message, num_bytes);
    mssg->inuse = 0;
    mssg->length = 0;
  }
  dbprintf("Read %d bytes\n", num_bytes);
  return num_bytes;
}

int MboxRecvInternal(Mbox* mbox, int maxlength, void* message) {
  Link* l;
  uint32 interrupts;
  MboxMessage* mssg;
  int dummy, dummy2;
  dbprintf('y', "MboxRecvInternal:Entering PID: %d\n", GetCurrentPid());
  RETURN_FAIL_IF_NULL(mssg);
  // GUARDED_SCOPE(mbox->mbox_lock, dummy) {
  //   l = AQueueFirst(&mbox->messages);
  //   mssg = (MboxMessage*)AQueueObject(l);
  // }
  // RETURN_FAIL_IF_FALSE(SanityCheckRequestedLength(mssg, maxlength));

  dbprintf('y', "MboxRecvInternal: Receiving message %s, PID: %d\n",
           mssg->message, GetCurrentPid());
  SemHandleWait(mbox->mbox_consumers_sem);
  GUARDED_SCOPE(mbox->mbox_lock, dummy) {
    l = AQueueFirst(&mbox->messages);
    mssg = (MboxMessage*)AQueueObject(l);
    // This is a weird case when you are guaranteed
    // that a consumer may consume something but
    // due to the invalidity that can only be inferred
    // when an item is present in the queue the consumer
    // doesn't end up consuming anything and thus resignals
    // the consumer semaphore.
    if (!SanityCheckRequestedLength(mssg, maxlength)) {
      SemHandleSignal(mbox->mbox_consumers_sem);
      // Unfortunately need to release the lock
      // manually because the `GUARDED_SCOPE` is being
      // exitted prematurely
      LockHandleRelease(mbox->mbox_lock);
      dbprintf('y', "PID: %d. Failed due to invalid length\n", GetCurrentPid());
      return MBOX_FAIL;
    }
    UNINTERRUPTIBLE_SCOPE(interrupts, dummy2) {
      if (AQueueRemove(&l) != QUEUE_SUCCESS) {
        printf(
            "FATAL ERROR: could not remove link from messages queue in "
            "MboxRecvInternal: PID: %d!\n",
            GetCurrentPid());
        exitsim();
      }
    }
  }
  SemHandleSignal(mbox->mbox_producers_sem);

  return ReadOutMessageData(mssg, maxlength, message);
}

int MboxRecv(mbox_t handle, int maxlength, void* message) {
  dbprintf('y', "MboxRecv:Entering , PID: %d\n", GetCurrentPid());
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
    if (MboxCloseInternal(&mboxes[handle], pid) != MBOX_SUCCESS) {
      dbprintf('y', "PID %d not in Mbox %d\n", pid, handle);
    }
  }
  return MBOX_SUCCESS;
}
