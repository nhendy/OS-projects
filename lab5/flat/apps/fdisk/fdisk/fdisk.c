#include "usertraps.h"
#include "misc.h"

#include "fdisk.h"
#include "os/disk.h"

dfs_superblock sb;
dfs_inode inodes[DFS_INODE_MAX_NUM];
uint32 fbv[DFS_FBV_MAX_NUM_WORDS];

int diskblocksize = 0;  // These are global in order to speed things up
int disksize = 0;       // (i.e. fewer traps to OS to get the same number)
static int negativeone = 0xFFFFFFFF;

int FdiskWriteBlock(uint32 blocknum, dfs_block *b);  // You can use your own
                                                     // function. This function
// calls disk_write_block() to write physical blocks to disk
void WriteBlockOrDie(uint32 blocknum, dfs_block *b);

static uint32 invert(uint32 n) { return (n ^ negativeone); }

void initSuperBlock() {
  Printf("Init'ing super block\n");
  sb.valid = 0;
  sb.dfs_block_size = DFS_BLOCKSIZE;
  sb.dfs_num_blocks = DFS_MAX_NUM_BLOCKS;
  sb.dfs_start_block_inodes = DFS_INODE_START_BLOCK;
  sb.dfs_num_inodes = DFS_INODE_MAX_NUM;
  sb.dfs_start_block_fbv = DFS_FBV_START_BLOCK;
  sb.dfs_start_block_data = DFS_DATA_START_BLOCK;
}

void SetFreeBlockVector(int block, int val) {
  uint32 wd = block / 32;
  uint32 bitnum = block % 32;

  fbv[wd] = (fbv[wd] & invert(1 << bitnum)) | (val << bitnum);
}

void main(int argc, char *argv[]) {
  // STUDENT: put your code here. Follow the guidelines below. They are just the
  // main steps.
  // You need to think of the finer details. You can use bzero() to zero out
  // bytes in memory

  dfs_block dfs_b;
  int i;
  // Initializations and argc check
  // argc

  // Need to invalidate filesystem before writing to it to make sure that the OS
  // doesn't wipe out what we do here with the old version in memory
  // You can use dfs_invalidate(); but it will be implemented in Problem 2. You
  // can just do
  initSuperBlock();
  Printf("Reading disk sizes\n");
  diskblocksize = disk_blocksize();
  disksize = disk_size();
  bzero(&dfs_b, sizeof(dfs_block));

  // /* // Make sure the disk exists before doing anything else */
  if (disk_create() == DISK_FAIL) {
    Printf("Failed to create disk\n");
    exit();
  }

  // // Write all inodes as not in use and empty (all zeros)
  Printf("%d\n", sb.dfs_start_block_fbv);
  for (i = sb.dfs_start_block_inodes; i < sb.dfs_start_block_fbv; ++i) {
    WriteBlockOrDie(i, &dfs_b);
  }

  // Next, setup free block vector (fbv) and write free block vector to the
  // disk
  bzero(&fbv, sizeof(fbv) / sizeof(fbv[0]));
  Printf("Setting fbv\n");
  for (i = sb.dfs_start_block_data; i < sb.dfs_num_blocks; ++i) {
    SetFreeBlockVector(i, 1);
  }

  Printf("Writing fbv from %d till %d. Size %d \n", sb.dfs_start_block_fbv,
         sb.dfs_start_block_data, DFS_FBV_MAX_NUM_WORDS);
  for (i = sb.dfs_start_block_fbv; i < sb.dfs_start_block_data; ++i) {
    bcopy(&((char *)fbv)[sizeof(dfs_block) * (i - sb.dfs_start_block_fbv)],
          dfs_b.data, sizeof(dfs_block));
    WriteBlockOrDie(i, &dfs_b);
  }

  // Finally, setup superblock as valid filesystem and write superblock and
  // boot
  sb.valid = 1;
  // record to disk:
  // boot record is all zeros in the first physical block, and superblock
  // structure goes into the second physical block
  bzero(dfs_b.data, sizeof(dfs_block));
  bcopy(&sb, &dfs_b.data[diskblocksize], sizeof(dfs_superblock));
  WriteBlockOrDie(0, &dfs_b);

  Printf("fdisk (%d): Formatted DFS disk for %d bytes.\n", getpid(), disksize);
}

void WriteBlockOrDie(uint32 blocknum, dfs_block *b) {
  if (FdiskWriteBlock(blocknum, b) == DISK_FAIL) {
    Printf("Failed to write dfs disk block %i\n", blocknum);
    exit();
  }
}

int FdiskWriteBlock(uint32 blocknum, dfs_block *b) {
  // STUDENT: put your code here
  disk_block db;
  int i, disk_blocks_per_df_block;

  bzero(&db, sizeof(disk_block));
  disk_blocks_per_df_block = sizeof(dfs_block) / sizeof(disk_block);
  Printf("Writing %d physical disk blocks, dfs block num %d\n",
         disk_blocks_per_df_block, blocknum);
  for (i = 0; i < disk_blocks_per_df_block; ++i) {
    bcopy(b, &db, sizeof(disk_block));
    if (disk_write_block(blocknum * disk_blocks_per_df_block + i, &db.data) ==
        DISK_FAIL) {
      Printf("Failed to write physical disk block %i\n", i);
      return DISK_FAIL;
    }
  }
  return DISK_SUCCESS;
}
