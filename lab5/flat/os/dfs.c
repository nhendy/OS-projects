#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "queue.h"
#include "disk.h"
#include "dfs.h"
#include "synch.h"

static dfs_inode inodes[DFS_INODE_MAX_NUM];  // all inodes
static dfs_superblock sb;                    // superblock
static uint32 fbv[DFS_FBV_MAX_NUM_WORDS];    // Free block vector

static lock_t fbv_lock;
static lock_t inode_lock;

static uint32 negativeone = 0xFFFFFFFF;
static inline uint32 invert(uint32 n) { return n ^ negativeone; }

#define DO_OR_DIE(result, death_val, death_format, args...) \
  do {                                                      \
    if (result == death_val) {                              \
      ELOG(death_format, ##args);                           \
      GracefulExit();                                       \
    }                                                       \
  } while (0)
#define CHECK_OR_DIE(result, death_format, args...) \
  do {                                              \
    if (!result) {                                  \
      ELOG(death_format, ##args);                   \
      GracefulExit();                               \
    }                                               \
  } while (0)
#define DO_OR_FAIL(result, death_val, death_format, args...) \
  do {                                                       \
    if (result == death_val) {                               \
      ELOG(death_format, ##args);                            \
      return death_val;                                      \
    }                                                        \
  } while (0)
#define LOCK_OR_FAIL(locking_function, death_val, death_format, args...) \
  do {                                                                   \
    if (locking_function == SYNC_FAIL) {                                 \
      ELOG(death_format, ##args);                                        \
      return death_val;                                                  \
    }                                                                    \
  } while (0)

#define CHECK_FS_VALID_OR_FAIL(format, args...) \
  do {                                          \
    if (!DfsIsValid()) {                        \
      ELOG(format, ##args);                     \
      return DFS_FAIL;                          \
    }                                           \
  } while (0)

#define DISK_DO_OR_DIE(result, death_message) \
  DO_OR_DIE(result, DISK_FAIL, death_message)
#define DFS_DO_OR_DIE(result, death_message) \
  DO_OR_DIE(result, DFS_FAIL, death_message)
#define DFS_DO_OR_FAIL(result, death_format, args...) \
  DO_OR_FAIL(result, DFS_FAIL, death_format, ##args)

#define BLOCK_AVAILABLE 1
#define BLOCK_TAKEN 0

// You have already been told about the most likely places where you should use
// locks. You may use
// additional locks if it is really necessary.

// STUDENT: put your file system level functions below.
// Some skeletons are provided. You can implement additional functions.

///////////////////////////////////////////////////////////////////
// Non-inode functions first
///////////////////////////////////////////////////////////////////

int DfsIsValid() { return sb.valid == 1; }
int DfsInodeIsValid(handle) {
  return handle >= 0 && handle < sizeof(inodes) / sizeof(inodes[0]);
}

void SetFreeBlockVector(int block, int val) {
  uint32 wd = block / 32;
  uint32 bitnum = block % 32;

  fbv[wd] = (fbv[wd] & invert(1 << bitnum)) | (val << bitnum);
}

int IsBlockAllocated(int block) {
  uint32 wd = block / 32;
  uint32 bitnum = block % 32;
  /* LOG("block %d: %d\n", block, fbv[wd] & (1 << bitnum)); */
  return (fbv[wd] & (1 << bitnum)) == 0;
}
//-----------------------------------------------------------------
// DfsModuleInit is called at boot time to initialize things and
// open the file system for use.
//-----------------------------------------------------------------

void DfsModuleInit() {
  // You essentially set the file system as invalid and then open
  // using DfsOpenFileSystem().
  DfsInvalidate();
  fbv_lock = LockCreate();
  inode_lock = LockCreate();
  LOG("Number of max dfs blocks %d\n", DFS_MAX_NUM_BLOCKS);
  DfsOpenFileSystem();
}

//-----------------------------------------------------------------
// DfsInavlidate marks the current version of the filesystem in
// memory as invalid.  This is really only useful when formatting
// the disk, to prevent the current memory version from overwriting
// what you already have on the disk when the OS exits.
//-----------------------------------------------------------------

void DfsInvalidate() {
  // This is just a one-line function which sets the valid bit of the
  // superblock to 0.
  sb.valid = 0;
}

void DumpSuperBlock() {
  disk_block db;
  bzero(db.data, sizeof(disk_block));
  bcopy(&sb, db.data, sizeof(dfs_superblock));
  DISK_DO_OR_DIE(DiskWriteBlock(1, &db),
                 "Failed to write superblock to disk\n");
}

//-------------------------------------------------------------------
// DfsOpenFileSystem loads the file system metadata from the disk
// into memory.  Returns DFS_SUCCESS on success, and DFS_FAIL on
// failure.
//-------------------------------------------------------------------

int DfsOpenFileSystem() {
  // Basic steps:
  // Check that filesystem is not already open

  disk_block db;
  dfs_block dfs_b;
  int i;

  if (DfsIsValid()) {
    LOG("File system  already open. Exiting..\n");
    return DFS_FAIL;
  }
  // Read superblock from disk.  Note this is using the disk read rather
  // than the DFS read function because the DFS read requires a valid
  // filesystem in memory already, and the filesystem cannot be valid
  // until we read the superblock. Also, we don't know the block size
  // until we read the superblock, either.
  if (DiskReadBlock(1, &db) == DISK_FAIL) {
    LOG("Failed to read first time. File might not exist. Retrying...\n");
    DISK_DO_OR_DIE(DiskCreate(),
                   "DfsOpenFileSystem: Failed to create disk  \n");
    DISK_DO_OR_DIE(DiskReadBlock(1, &db),
                   "DfsOpenFileSystem: Failed to read block 1 \n");
  }

  // Copy the data from the block we just read into the superblock in memory
  bcopy(db.data, &sb, sizeof(dfs_superblock));
  if (!DfsIsValid()) {
    LOG("Filesystem still invalid. Exiting..\n");
    return DFS_FAIL;
  }

  // All other blocks are sized by virtual block size:
  // Read inodes
  for (i = sb.dfs_start_block_inodes; i < sb.dfs_start_block_fbv; ++i) {
    DFS_DO_OR_FAIL(DfsReadBlock(i, &dfs_b),
                   "DfsOpenFileSystem: Failed to read DFS inode block\n");
    bcopy(dfs_b.data,
          &(((char *)
             inodes)[sizeof(dfs_block) * (i - sb.dfs_start_block_inodes)]),
          sizeof(dfs_block));
  }
  // Read free block vector
  for (i = sb.dfs_start_block_fbv; i < sb.dfs_start_block_data; ++i) {
    DFS_DO_OR_FAIL(DfsReadBlock(i, &dfs_b),
                   "DfsOpenFileSystem: Failed to read DFS fbv block\n");
    bcopy(dfs_b.data,
          &(((char *)fbv)[sizeof(dfs_block) * (i - sb.dfs_start_block_fbv)]),
          sizeof(dfs_block));
  }
  // Change superblock to be invalid, write back to disk, then change
  // it back to be valid in memory
  DfsInvalidate();
  DumpSuperBlock();
  sb.valid = 1;
  LOG("DFS is valid %d\n", sb.valid);
  return DFS_SUCCESS;
}

//-------------------------------------------------------------------
// DfsCloseFileSystem writes the current memory version of the
// filesystem metadata to the disk, and invalidates the memory's
// version.
//-------------------------------------------------------------------

int DfsCloseFileSystem() {
  dfs_block dfs_b;
  int i;

  CHECK_FS_VALID_OR_FAIL("File system  already closed. Failing..\n");
  for (i = sb.dfs_start_block_inodes; i < sb.dfs_start_block_fbv; ++i) {
    bcopy(
        &((char *)inodes)[sizeof(dfs_block) * (i - sb.dfs_start_block_inodes)],
        dfs_b.data, sizeof(dfs_block));
    DFS_DO_OR_FAIL(DfsWriteBlock(i, &dfs_b),
                   "DfsOpenFileSystem: Failed to read DFS inode block\n");
  }
  for (i = sb.dfs_start_block_fbv; i < sb.dfs_start_block_data; ++i) {
    bcopy(&((char *)fbv)[sizeof(dfs_block) * (i - sb.dfs_start_block_fbv)],
          dfs_b.data, sizeof(dfs_block));
    DFS_DO_OR_FAIL(DfsWriteBlock(i, &dfs_b),
                   "DfsOpenFileSystem: Failed to read DFS fbv block\n");
  }

  DumpSuperBlock();
  DfsInvalidate();
  return DFS_SUCCESS;
}

//-----------------------------------------------------------------
// DfsAllocateBlock allocates a DFS block for use. Remember to use
// locks where necessary.
//-----------------------------------------------------------------

uint32 DfsAllocateBlock() {
  // Check that file system has been validly loaded into memory
  // Find the first free block using the free block vector (FBV), mark it in use
  // Return handle to block
  //
  int i;
  int bitnum;
  CHECK_FS_VALID_OR_FAIL("File system  already closed. Failing..\n");
  // Acquire lock
  LOCK_OR_FAIL(LockHandleAcquire(fbv_lock), DFS_FAIL,
               "Failed to acquire fbv_lock.\n");

  // Find non zero word
  for (i = 0; fbv[i] == BLOCK_TAKEN && i < sizeof(fbv) / sizeof(fbv[0]); ++i)
    ;

  // If more than fbv size then no words ae free
  if (i == sizeof(fbv) / sizeof(fbv[0])) {
    LOCK_OR_FAIL(LockHandleRelease(fbv_lock), DFS_FAIL,
                 "Failed to release lock\n");
    LOG("No free blocks\n");
    /* GracefulExit(); */
    return DFS_FAIL;
  }

  // Find non zero bit
  for (bitnum = 0; (fbv[i] & (1 << bitnum)) == BLOCK_TAKEN; ++bitnum)
    ;
  // Set bit to 0
  fbv[i] &= invert(1 << bitnum);
  // Release lock
  LOCK_OR_FAIL(LockHandleRelease(fbv_lock), DFS_FAIL,
               "Failed to release lock\n");
  DLOG('f', "Allocated dfs block %d\n", i * 32 + bitnum);

  return (i * 32 + bitnum);
}

//-----------------------------------------------------------------
// DfsFreeBlock deallocates a DFS block.
//-----------------------------------------------------------------

int DfsFreeBlock(uint32 blocknum) {

  CHECK_FS_VALID_OR_FAIL(" File system  already closed. Failing..\n");
  if (blocknum > DFS_MAX_NUM_BLOCKS) {
    LOG("Blocknum is greater than max num blocks :%d\n", DFS_MAX_NUM_BLOCKS);
    return DFS_FAIL;
  }
  if (!IsBlockAllocated(blocknum)) {
    LOG("Block %d is not allocated\n");
    /* GracefulExit(); */
    return DFS_FAIL;
  }
  LOCK_OR_FAIL(LockHandleAcquire(fbv_lock), DFS_FAIL,
               "Failed to acquire fbv_lock.\n");
  SetFreeBlockVector(blocknum, BLOCK_AVAILABLE);
  LOCK_OR_FAIL(LockHandleRelease(fbv_lock), DFS_FAIL,
               "Failed to release lock\n");
  return DFS_SUCCESS;
}

//-----------------------------------------------------------------
// DfsReadBlock reads an allocated DFS block from the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to read from it.  Returns DFS_FAIL
// on failure, and the number of bytes read on success.
//-----------------------------------------------------------------

int DfsReadBlock(uint32 blocknum, dfs_block *b) {
  int i, disk_blocks_per_dfs_block = sb.dfs_block_size / DiskBytesPerBlock();
  dfs_block dfs_b;
  disk_block db;
  CHECK_FS_VALID_OR_FAIL("File system  already closed. Failing..\n");
  if (!IsBlockAllocated(blocknum)) {
    LOG("Block %d is not allocated\n");
    return DFS_FAIL;
  }
  bzero(&dfs_b, sizeof(dfs_block));
  for (i = 0; i < disk_blocks_per_dfs_block; ++i) {
    DISK_DO_OR_DIE(DiskReadBlock(i + blocknum * disk_blocks_per_dfs_block, &db),
                   "Failed to read block from disk\n");
    bcopy(db.data, &(dfs_b.data[i * sizeof(disk_block)]), sizeof(disk_block));
  }

  bcopy(&dfs_b, b, sizeof(dfs_block));
  return sb.dfs_block_size;
}

//-----------------------------------------------------------------
// DfsWriteBlock writes to an allocated DFS block on the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to write to it.  Returns DFS_FAIL
// on failure, and the number of bytes written on success.
//-----------------------------------------------------------------

int DfsWriteBlock(uint32 blocknum, dfs_block *b) {
  int i, disk_blocks_per_dfs_block = sb.dfs_block_size / DiskBytesPerBlock();
  disk_block db;
  CHECK_FS_VALID_OR_FAIL("File system  already closed. Failing..\n");
  if (!IsBlockAllocated(blocknum)) {
    printf("Block %d is not allocated\n");
    return DFS_FAIL;
  }
  for (i = 0; i < disk_blocks_per_dfs_block; ++i) {
    bcopy(&(b->data[i * sizeof(disk_block)]), db.data, sizeof(disk_block));
    DLOG('f', "Writing to block %d\n",
         i + blocknum * disk_blocks_per_dfs_block);
    DISK_DO_OR_DIE(
        DiskWriteBlock(i + blocknum * disk_blocks_per_dfs_block, &db),
        "Failed to read block from disk\n");
  }
  return sb.dfs_block_size;
}

////////////////////////////////////////////////////////////////////////////////
// Inode-based functions
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------
// DfsInodeFilenameExists looks through all the inuse inodes for
// the given filename. If the filename is found, return the handle
// of the inode. If it is not found, return DFS_FAIL.
//-----------------------------------------------------------------

uint32 DfsInodeFilenameExists(char *filename) {
  int i;

  CHECK_FS_VALID_OR_FAIL("File system  already closed. Failing..\n");
  if (dstrlen(filename) > DFS_INODE_MAX_FILENAME) return DFS_FAIL;
  for (i = 0; i < sizeof(inodes) / sizeof(inodes[0]); ++i) {
    if (inodes[i].inuse &&
        dstrncmp(filename, inodes[i].filename, dstrlen(filename)) == 0) {
      return i;
    }
  }
  return DFS_FAIL;
}

//-----------------------------------------------------------------
// DfsInodeOpen: search the list of all inuse inodes for the
// specified filename. If the filename exists, return the handle
// of the inode. If it does not, allocate a new inode for this
// filename and return its handle. Return DFS_FAIL on failure.
// Remember to use locks whenever you allocate a new inode.
//-----------------------------------------------------------------

uint32 DfsInodeOpen(char *filename) {
  uint32 handle;
  CHECK_FS_VALID_OR_FAIL("File system  already closed. Failing..\n");
  if (dstrlen(filename) > DFS_INODE_MAX_FILENAME) return DFS_FAIL;
  if ((handle = DfsInodeFilenameExists(filename)) != DFS_FAIL) {
    DLOG('f', "Returning handle %d\n", handle);
    return handle;
  }

  LOCK_OR_FAIL(LockHandleAcquire(inode_lock), DFS_FAIL,
               "Failed to acquire inode_lock.\n");
  for (handle = 0;
       handle < sizeof(inodes) / sizeof(inodes[0]) && inodes[handle].inuse;
       ++handle)
    ;
  if (handle == sizeof(inodes) / sizeof(inodes[0])) {
    LOCK_OR_FAIL(LockHandleRelease(inode_lock), DFS_FAIL,
                 "Failed to release inode_lock.\n");
    LOG("All inodes inuse\n");
    return DFS_FAIL;
  }
  bcopy(filename, inodes[handle].filename, DFS_INODE_MAX_FILENAME);
  inodes[handle].inuse = 1;
  inodes[handle].size = 0;
  LOCK_OR_FAIL(LockHandleRelease(inode_lock), DFS_FAIL,
               "Failed to release inode_lock.\n");
  return handle;
}

//-----------------------------------------------------------------
// DfsInodeDelete de-allocates any data blocks used by this inode,
// including the indirect addressing block if necessary, then mark
// the inode as no longer in use. Use locks when modifying the
// "inuse" flag in an inode.Return DFS_FAIL on failure, and
// DFS_SUCCESS on success.
//-----------------------------------------------------------------

int DfsInodeDelete(uint32 handle) {
  int i;
  dfs_block dfs_b;
  block_idx_t *indirect_table;
  CHECK_FS_VALID_OR_FAIL("File system  already closed. Failing..\n");
  if (!DfsInodeIsValid(handle)) return DFS_FAIL;

  if (!inodes[handle].inuse) {
    printf("The inode is not inuse\n");
    return DFS_FAIL;
  }
  LOCK_OR_FAIL(LockHandleAcquire(inode_lock), DFS_FAIL,
               "Failed to acquire inode_lock.\n");
  // Free virtual blocks
  for (i = 0; i < sizeof(inodes[handle].virtual_blocks) /
                      sizeof(inodes[handle].virtual_blocks[0]);
       ++i) {
    if (inodes[handle].virtual_blocks[i] != 0)
      DFS_DO_OR_FAIL(DfsFreeBlock(inodes[handle].virtual_blocks[i]),
                     "Failed to free virtual block %d dfs block %d\n", i,
                     inodes[handle].virtual_blocks[i]);
  }
  if (inodes[handle].indirect_block != 0) {
    // Read block containing indirect blocks
    DFS_DO_OR_FAIL(DfsReadBlock(inodes[handle].indirect_block, &dfs_b),
                   "Failed to read indirect block\n");
    indirect_table = (block_idx_t *)dfs_b.data;
    // Free indirect blocks
    for (i = 0; i < sizeof(dfs_block) / sizeof(block_idx_t); ++i) {
      // Read sizeof(block_idx_t) at a time
      if (indirect_table[i] != 0)
        DFS_DO_OR_FAIL(DfsFreeBlock(indirect_table[i]),
                       "Failed to free indirect block\n");
    }
  }
  // Zero out inode
  bzero(&inodes[handle], sizeof(dfs_inode));
  LOCK_OR_FAIL(LockHandleRelease(inode_lock), DFS_FAIL,
               "Failed to release inode_lock.\n");
  return DFS_SUCCESS;
}

void PrintArray(char *arry, int n) {
  int i;
  for (i = 0; i < n; ++i) printf("%c\n", arry[i]);
}
//-----------------------------------------------------------------
// DfsInodeReadBytes reads num_bytes from the file represented by
// the inode handle, starting at virtual byte start_byte, copying
// the data to the address pointed to by mem. Return DFS_FAIL on
// failure, and the number of bytes read on success.
//-----------------------------------------------------------------

int DfsInodeReadBytes(uint32 handle, void *mem, int start_byte, int num_bytes) {
  dfs_block dfs_b;
  block_idx_t virtual_block;
  int cursor = start_byte;
  int eof = start_byte + num_bytes;
  uint32 bytes_to_read;
  char *mem_bytes = (char *)mem;

  CHECK_FS_VALID_OR_FAIL(" File system  already closed. Failing..\n");
  if (!DfsInodeIsValid(handle)) return DFS_FAIL;
  if (!inodes[handle].inuse) return DFS_FAIL;
  // TODO: (nhendy) > or >=
  if (eof >= DFS_MAX_FILESYSTEM_SIZE) return DFS_FAIL;

  while (cursor < eof) {
    if ((virtual_block = DfsInodeTranslateVirtualToFilesys(
             handle, cursor / sb.dfs_block_size)) == DFS_FAIL) {
      LOG("Failed to translate block\n");
      return DFS_FAIL;
    }
    DLOG('f', "Cursor: %d, start_byte: %d, eof %d, offset %d , block %d\n",
         cursor, start_byte, eof, cursor % sb.dfs_block_size,
         cursor / sb.dfs_block_size);
    if (virtual_block == 0) return DFS_FAIL;

    DFS_DO_OR_FAIL(DfsReadBlock(virtual_block, &dfs_b),
                   "Failed to read virtual block\n");
    bytes_to_read =
        min(eof - cursor, sb.dfs_block_size - cursor % sb.dfs_block_size);
    // TODO: (nhendy) needs testing
    bcopy(dfs_b.data + cursor % sb.dfs_block_size,
          &(mem_bytes[cursor - start_byte]), bytes_to_read);
    cursor += bytes_to_read;
  }
  return cursor - start_byte;
}

//-----------------------------------------------------------------
// DfsInodeWriteBytes writes num_bytes from the memory pointed to
// by mem to the file represented by the inode handle, starting at
// virtual byte start_byte. Note that if you are only writing part
// of a given file system block, you'll need to read that block
// from the disk first. Return DFS_FAIL on failure and the number
// of bytes written on success.
//-----------------------------------------------------------------

int DfsInodeWriteBytes(uint32 handle, void *mem, int start_byte,
                       int num_bytes) {
  dfs_block dfs_b;
  block_idx_t virtual_block;
  int cursor = start_byte;
  int eof = start_byte + num_bytes;
  uint32 bytes_to_write;
  char *mem_bytes = (char *)mem;
  CHECK_FS_VALID_OR_FAIL("File system  already closed. Failing..\n");

  if (!DfsInodeIsValid(handle)) return DFS_FAIL;
  if (!inodes[handle].inuse) return DFS_FAIL;
  if (start_byte < 0) return DFS_FAIL;
  if (num_bytes < 0) return DFS_FAIL;
  // TODO: (nhendy) > or >=
  if (eof >= DFS_MAX_FILESYSTEM_SIZE) return DFS_FAIL;
  while (cursor < eof) {
    // This will return if block is already allocated.
    // instead of attempting to translate then allocating
    DFS_DO_OR_FAIL((virtual_block = DfsInodeAllocateVirtualBlock(
                        handle, cursor / sb.dfs_block_size)),
                   "Failed to allocate virtual block\n");

    DFS_DO_OR_FAIL(DfsReadBlock(virtual_block, &dfs_b),
                   "Failed to read virtual block\n");

    bzero(dfs_b.data, sizeof(dfs_block));

    // TODO: (nhendy) needs testing
    bytes_to_write =
        min(eof - cursor, sb.dfs_block_size - cursor % sb.dfs_block_size);

    bcopy(&(mem_bytes[cursor - start_byte]),
          dfs_b.data + cursor % sb.dfs_block_size, bytes_to_write);
    DLOG('f',
         "Cursor: %d, start_byte: %d, eof %d, offset %d, bytes to write %d\n",
         cursor, start_byte, eof, cursor % sb.dfs_block_size, bytes_to_write);

    cursor += bytes_to_write;
    DLOG('f', "Writing bytes to %d\n", virtual_block);
    DFS_DO_OR_FAIL(DfsWriteBlock(virtual_block, &dfs_b),
                   "Failed to write virtual block\n");
  }
  CHECK_OR_DIE(eof == cursor, "Must end at eof!! Exiting...");
  inodes[handle].size = eof;
  return cursor - start_byte;
}

//-----------------------------------------------------------------
// DfsInodeFilesize simply returns the size of an inode's file.
// This is defined as the maximum virtual byte number that has
// been written to the inode thus far. Return DFS_FAIL on failure.
//-----------------------------------------------------------------

uint32 DfsInodeFilesize(uint32 handle) {
  if (!DfsInodeIsValid(handle)) return DFS_FAIL;
  if (!inodes[handle].inuse) return DFS_FAIL;
  DLOG('f', "size %d\n", inodes[handle].size);
  return inodes[handle].size;
}

//-----------------------------------------------------------------
// DfsInodeAllocateVirtualBlock allocates a new filesystem block
// for the given inode, storing its blocknumber at index
// virtual_blocknumber in the translation table. If the
// virtual_blocknumber resides in the indirect address space, and
// there is not an allocated indirect addressing table, allocate it.
// Return DFS_FAIL on failure, and the newly allocated file system
// block number on success.
//-----------------------------------------------------------------

uint32 DfsInodeAllocateVirtualBlock(uint32 handle, uint32 virtual_blocknum) {
  dfs_block dfs_b;
  uint32 block;
  if (!DfsInodeIsValid(handle)) return DFS_FAIL;
  if (!inodes[handle].inuse) return DFS_FAIL;
  if (virtual_blocknum >=
      NUM_VIRTUAL_BLOCKS + sizeof(dfs_block) / sizeof(block_idx_t))
    return DFS_FAIL;
  if (virtual_blocknum < NUM_VIRTUAL_BLOCKS) {
    if (inodes[handle].virtual_blocks[virtual_blocknum] != 0)
      return inodes[handle].virtual_blocks[virtual_blocknum];
    return (inodes[handle].virtual_blocks[virtual_blocknum] =
                DfsAllocateBlock());
  }

  if (inodes[handle].indirect_block == 0) {
    DLOG('f', "Allocating indirect block\n");
    inodes[handle].indirect_block = DfsAllocateBlock();
    bzero(&dfs_b, sizeof(dfs_block));
  } else {
    DFS_DO_OR_FAIL(DfsReadBlock(inodes[handle].indirect_block, &dfs_b),
                   "Failed to read indirect block\n");
  }
  ((block_idx_t *)dfs_b.data)[virtual_blocknum - NUM_VIRTUAL_BLOCKS] =
      DfsAllocateBlock();
  DFS_DO_OR_FAIL(DfsWriteBlock(inodes[handle].indirect_block, &dfs_b),
                 "DfsOpenFileSystem: Failed to read DFS inode block\n");
  return ((block_idx_t *)dfs_b.data)[virtual_blocknum - NUM_VIRTUAL_BLOCKS];
}

//-----------------------------------------------------------------
// DfsInodeTranslateVirtualToFilesys translates the
// virtual_blocknum to the corresponding file system block using
// the inode identified by handle. Return DFS_FAIL on failure.
//-----------------------------------------------------------------

uint32 DfsInodeTranslateVirtualToFilesys(uint32 handle,
                                         uint32 virtual_blocknum) {
  dfs_block dfs_b;
  if (!DfsInodeIsValid(handle)) return DFS_FAIL;
  if (virtual_blocknum >=
      NUM_VIRTUAL_BLOCKS + sizeof(dfs_block) / sizeof(block_idx_t))
    return DFS_FAIL;

  // If it's a direct virtual block
  if (virtual_blocknum < NUM_VIRTUAL_BLOCKS) {
    return inodes[handle].virtual_blocks[virtual_blocknum];
  }
  // If indirect block is not allocated then fail
  if (inodes[handle].indirect_block == 0) return DFS_FAIL;
  // Read block containing indirect blocks
  DFS_DO_OR_FAIL(DfsReadBlock(inodes[handle].indirect_block, &dfs_b),
                 "Failed to read indirect block\n");
  return ((block_idx_t *)dfs_b.data)[virtual_blocknum - NUM_VIRTUAL_BLOCKS];
}
