#ifndef __MBOX_OS__
#define __MBOX_OS__
#include "synch.h"
// Maximum number of mailboxes allowed in the system
#define MBOX_NUM_MBOXES 16
// Maximum number of message buffers allowed in the system
#define MBOX_NUM_BUFFERS 50
// Maximum number of buffer slots available to any given mailbox
#define MBOX_MAX_BUFFERS_PER_MBOX 10
// Buffer size of 100 for each message
#define MBOX_MAX_MESSAGE_LENGTH 100

#define MBOX_FAIL -1
#define MBOX_SUCCESS 1

//---------------------------------------------
// Define your mailbox structures here
//--------------------------------------------

typedef struct {
  char message[MBOX_MAX_MESSAGE_LENGTH + 1];
  int length;
  uint32 inuse;
  lock_t mssg_lock;
} MboxMessage;

typedef struct {
  Queue messages;
  uint32 inuse;
  Queue pids;
  lock_t mbox_lock;
  sem_t mbox_producers_sem;
  sem_t mbox_consumers_sem;
} Mbox;

static Mbox mboxes[MBOX_NUM_MBOXES];
static MboxMessage mboxes_messages[MBOX_NUM_BUFFERS];

typedef int mbox_t;       // This is the "type" of mailbox handles
typedef int mbox_mssg_t;  // This is the "type" of mailbox message handles

//-------------------------------------------
// Prototypes for Mbox functions you have to write
//-------------------------------------------

void MboxModuleInit();
mbox_t MboxCreate();
int MboxOpen(mbox_t m);
int MboxClose(mbox_t m);
int MboxSend(mbox_t m, int length, void *message);
int MboxRecv(mbox_t m, int maxlength, void *message);
int MboxCloseAllByPid(int pid);

#ifndef false
#define false 0
#endif

#ifndef true
#define true 1
#endif

#endif
