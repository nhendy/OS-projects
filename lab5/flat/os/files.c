#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "dfs.h"
#include "files.h"
#include "synch.h"

// You have already been told about the most likely places where you should use
// locks. You may use
// additional locks if it is really necessary.
static file_descriptor fds[FILE_MAX_OPEN_FILES];
static lock_t fd_lock;
#define DO_OR_DIE(result, death_val, death_format, args...) \
  do {                                                      \
    if (result == death_val) {                              \
      printf(death_format, ##args);                         \
      GracefulExit();                                       \
    }                                                       \
  } while (0)

#define LOCK_OR_FAIL(locking_function, death_val, death_format, args...) \
  do {                                                                   \
    if (locking_function == SYNC_FAIL) {                                 \
      printf(death_format, ##args);                                      \
      return death_val;                                                  \
    }                                                                    \
  } while (0)

// STUDENT: put your file-level functions here
int FileDescFilenameExists(char *name) {
  int i;
  while (dstrncmp(fds[i].filename, name, dstrlen(name)) != 0 &&
         i < FILE_MAX_OPEN_FILES)
    i++;
  return i < FILE_MAX_OPEN_FILES;
}

uint32 FileAllocateDescriptor(char *filename) {
  int i;

  LOCK_OR_FAIL(LockHandleAcquire(fd_lock), FILE_FAIL,
               "Failed to acquire fd_lock.\n");
  if (FileDescFilenameExists(filename)) {
    LOCK_OR_FAIL(LockHandleRelease(fd_lock), FILE_FAIL,
                 "Failed to release fd_lock\n");
    return FILE_FAIL;
  }

  while (fds[i].inuse != 0 && i < FILE_MAX_OPEN_FILES) i++;
  if (i > FILE_MAX_OPEN_FILES) {
    LOCK_OR_FAIL(LockHandleRelease(fd_lock), FILE_FAIL,
                 "Failed to release fd_lock\n");
    GracefulExit();
  }
  fds[i].inuse = 1;

  LOCK_OR_FAIL(LockHandleRelease(fd_lock), FILE_FAIL,
               "Failed to release fd_lock\n");
  return i;
}

Modes strToMode(char *mode) {
  if (dstrlen(mode) > kModeMaxStrLength) return INVALID;
  if (dstrlen(mode) == kModeMaxStrLength) return READWRITE;
  switch (mode[0]) {
    case 'r':
      return READ;
    case 'w':
      return WRITE;
    default:
      return INVALID;
  }
  return INVALID;
}

uint32 FileOpen(char *filename, char *mode) {
  uint32 descriptor;
  uint32 mode;
  uint32 handle;
  if ((descriptor = FileAllocateDescriptor(filename)) == FILE_FAIL)
    return FILE_FAIL;
  if ((mode = strToMode(mode)) == INVALID) return FILE_FAIL;
  // Init descriptor
  bzero(&fds[descriptor], sizeof(file_descriptor));
  // TODO: (nhendy) this implies deleting a file whenever attempting to write
  // to it (i.e we can't append or overwrite a file)
  if ((handle = DfsInodeFilenameExists(filename)) != DFS_FAIL &&
      (mode == WRITE || mode == READWRITE))
    DfsInodeDelete(handle);
  // Set inode handle
  DO_OR_DIE((fds[descriptor].inode_handle = DfsInodeOpen(filename)), DFS_FAIL,
            "[%s, %d]: Failed to open an inode\n", __FUNCTION__, __LINE__);
  // Copy filename
  dstrncpy(filename, fds[descriptor].filename, dstrlen(filename));
  // Set mode
  fds[descriptor].mode = mode;
  // Set pid
  fds[descriptor].pid = GetCurrentPid();

  return FILE_SUCCESS;
}

int FileClose(int handle) { return -1; }
int FileRead(int handle, void *mem, int num_bytes) { return -1; }
int FileWrite(int handle, void *mem, int num_bytes) { return -1; }
int FileSeek(int handle, int num_bytes, int from_where) { return -1; }
int FileDelete(char *filename) { return -1; }
