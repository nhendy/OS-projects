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

// STUDENT: put your file-level functions here
uint32 FileOpen(char *filename, char *mode) { return 0; }
int FileClose(int handle) { return -1; }
int FileRead(int handle, void *mem, int num_bytes) { return -1; }
int FileWrite(int handle, void *mem, int num_bytes) { return -1; }
int FileSeek(int handle, int num_bytes, int from_where) { return -1; }
int FileDelete(char *filename) { return -1; }
