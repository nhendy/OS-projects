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

static uint32 negativeone = 0xFFFFFFFF;
static inline uint32 invert(uint32 n) { return n ^ negativeone; }

#define DO_OR_DIE(result, death_val, death_message) \
  do {                                              \
    if (result == death_val) {                      \
      printf(death_message);                        \
      GracefulExit();                               \
    }                                               \
  } while (0)
#define DO_OR_FAIL(result, death_val, death_message) \
  do {                                               \
    if (result == death_val) {                       \
      printf(death_message);                         \
      return death_val;                              \
    }                                                \
  } while (0)

#define DISK_DO_OR_DIE(result, death_message) \
  DO_OR_DIE(result, DISK_FAIL, death_message)
#define DFS_DO_OR_DIE(result, death_message) \
  DO_OR_DIE(result, DFS_FAIL, death_message)
#define DFS_DO_OR_FAIL(result, death_message) \
  DO_OR_FAIL(result, DFS_FAIL, death_message)
// You have already been told about the most likely places where you should use
// locks. You may use
// additional locks if it is really necessary.

// STUDENT: put your file system level functions below.
// Some skeletons are provided. You can implement additional functions.

///////////////////////////////////////////////////////////////////
// Non-inode functions first
///////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------
// DfsModuleInit is called at boot time to initialize things and
// open the file system for use.
//-----------------------------------------------------------------

void DfsModuleInit() {
  // You essentially set the file system as invalid and then open
  // using DfsOpenFileSystem().
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

int DfsIsValid() { return sb.valid == 1; }

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
    printf("File system  already open. Exiting..\n");
    return DFS_FAIL;
  }
  // Read superblock from disk.  Note this is using the disk read rather
  // than the DFS read function because the DFS read requires a valid
  // filesystem in memory already, and the filesystem cannot be valid
  // until we read the superblock. Also, we don't know the block size
  // until we read the superblock, either.
  DISK_DO_OR_DIE(DiskReadBlock(1, &db),
                 "DfsOpenFileSystem: Failed to read block 1 \n");

  // Copy the data from the block we just read into the superblock in memory
  bcopy(db.data, &sb, sizeof(dfs_superblock));
  if (!DfsIsValid()) {
    printf("Filesystem still invalid. Exiting..\n");
    return DFS_FAIL;
  }

  // All other blocks are sized by virtual block size:
  // Read inodes
  for (i = sb.dfs_start_block_inodes; i < sb.dfs_start_block_fbv; ++i) {
    DFS_DO_OR_FAIL(DfsReadBlock(i, &dfs_b),
                   "DfsOpenFileSystem: Failed to read DFS inode block\n");
    bcopy(dfs_b.data,
          ((char *)inodes)[sizeof(dfs_block) * (i - sb.dfs_start_block_inodes)],
          sizeof(dfs_block));
  }
  // Read free block vector
  for (i = sb.dfs_start_block_fbv; i < sb.dfs_start_block_data; ++i) {
    DFS_DO_OR_FAIL(DfsReadBlock(i, &dfs_b),
                   "DfsOpenFileSystem: Failed to read DFS fbv block\n");
    bcopy(dfs_b.data,
          ((char *)fbv)[sizeof(dfs_block) * (i - sb.dfs_start_block_fbv)],
          sizeof(dfs_block));
  }
  // Change superblock to be invalid, write back to disk, then change
  // it back to be valid in memory
  DfsInvalidate();
  DumpSuperBlock();
  sb.valid = 1;
  return DFS_SUCCESS;
}

//-------------------------------------------------------------------
// DfsCloseFileSystem writes the current memory version of the
// filesystem metadata to the disk, and invalidates the memory's
// version.
//-------------------------------------------------------------------

int DfsCloseFileSystem() {
  disk_block db;
  dfs_block dfs_b;
  int i;

  if (!DfsIsValid()) {
    printf("File system  already closed. Exiting..\n");
    return DFS_FAIL;
  }
  for (i = sb.dfs_start_block_inodes; i < sb.dfs_start_block_fbv; ++i) {
    bcopy(((char *)inodes)[sizeof(dfs_block) * (i - sb.dfs_start_block_inodes)],
          dfs_b.data, sizeof(dfs_block));
    DFS_DO_OR_FAIL(DfsWriteBlock(i, &dfs_b),
                   "DfsOpenFileSystem: Failed to read DFS inode block\n");
  }
  for (i = sb.dfs_start_block_fbv; i < sb.dfs_start_block_data; ++i) {
    bcopy(((char *)fbv)[sizeof(dfs_block) * (i - sb.dfs_start_block_fbv)],
          dfs_b.data, sizeof(dfs_block));
    DFS_DO_OR_FAIL(DfsWriteBlock(i, &dfs_b),
                   "DfsOpenFileSystem: Failed to read DFS fbv block\n");
  }

  DumpSuperBlock();
  DfsInvalidate();
  return DFS_FAIL;
}

//-----------------------------------------------------------------
// DfsAllocateBlock allocates a DFS block for use. Remember to use
// locks where necessary.
//-----------------------------------------------------------------

uint32 DfsAllocateBlock() {
  // Check that file system has been validly loaded into memory
  // Find the first free block using the free block vector (FBV), mark it in use
  // Return handle to block
}

//-----------------------------------------------------------------
// DfsFreeBlock deallocates a DFS block.
//-----------------------------------------------------------------

int DfsFreeBlock(uint32 blocknum) {}

//-----------------------------------------------------------------
// DfsReadBlock reads an allocated DFS block from the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to read from it.  Returns DFS_FAIL
// on failure, and the number of bytes read on success.
//-----------------------------------------------------------------

int DfsReadBlock(uint32 blocknum, dfs_block *b) {}

//-----------------------------------------------------------------
// DfsWriteBlock writes to an allocated DFS block on the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to write to it.  Returns DFS_FAIL
// on failure, and the number of bytes written on success.
//-----------------------------------------------------------------

int DfsWriteBlock(uint32 blocknum, dfs_block *b) {}

////////////////////////////////////////////////////////////////////////////////
// Inode-based functions
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------
// DfsInodeFilenameExists looks through all the inuse inodes for
// the given filename. If the filename is found, return the handle
// of the inode. If it is not found, return DFS_FAIL.
//-----------------------------------------------------------------

uint32 DfsInodeFilenameExists(char *filename) {}

//-----------------------------------------------------------------
// DfsInodeOpen: search the list of all inuse inodes for the
// specified filename. If the filename exists, return the handle
// of the inode. If it does not, allocate a new inode for this
// filename and return its handle. Return DFS_FAIL on failure.
// Remember to use locks whenever you allocate a new inode.
//-----------------------------------------------------------------

uint32 DfsInodeOpen(char *filename) {}

//-----------------------------------------------------------------
// DfsInodeDelete de-allocates any data blocks used by this inode,
// including the indirect addressing block if necessary, then mark
// the inode as no longer in use. Use locks when modifying the
// "inuse" flag in an inode.Return DFS_FAIL on failure, and
// DFS_SUCCESS on success.
//-----------------------------------------------------------------

int DfsInodeDelete(uint32 handle) {}

//-----------------------------------------------------------------
// DfsInodeReadBytes reads num_bytes from the file represented by
// the inode handle, starting at virtual byte start_byte, copying
// the data to the address pointed to by mem. Return DFS_FAIL on
// failure, and the number of bytes read on success.
//-----------------------------------------------------------------

int DfsInodeReadBytes(uint32 handle, void *mem, int start_byte, int num_bytes) {
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
                       int num_bytes) {}

//-----------------------------------------------------------------
// DfsInodeFilesize simply returns the size of an inode's file.
// This is defined as the maximum virtual byte number that has
// been written to the inode thus far. Return DFS_FAIL on failure.
//-----------------------------------------------------------------

uint32 DfsInodeFilesize(uint32 handle) {}

//-----------------------------------------------------------------
// DfsInodeAllocateVirtualBlock allocates a new filesystem block
// for the given inode, storing its blocknumber at index
// virtual_blocknumber in the translation table. If the
// virtual_blocknumber resides in the indirect address space, and
// there is not an allocated indirect addressing table, allocate it.
// Return DFS_FAIL on failure, and the newly allocated file system
// block number on success.
//-----------------------------------------------------------------

uint32 DfsInodeAllocateVirtualBlock(uint32 handle, uint32 virtual_blocknum) {}

//-----------------------------------------------------------------
// DfsInodeTranslateVirtualToFilesys translates the
// virtual_blocknum to the corresponding file system block using
// the inode identified by handle. Return DFS_FAIL on failure.
//-----------------------------------------------------------------

uint32 DfsInodeTranslateVirtualToFilesys(uint32 handle,
                                         uint32 virtual_blocknum) {}
