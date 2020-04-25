#ifndef __DFS_SHARED__
#define __DFS_SHARED__

// Aliasing types for inode variables
typedef uint32 block_idx_t;
typedef char inuse_t;
typedef uint32 filesize_t;

typedef struct dfs_superblock {
  // STUDENT: put superblock internals here
  int valid;
  int dfs_block_size;
  int dfs_num_blocks;
  int dfs_start_block_inodes;
  int dfs_num_inodes;
  int dfs_start_block_fbv;
  int dfs_start_block_data;
} dfs_superblock;

#define DFS_BLOCKSIZE 1024  // Must be an integer multiple of the disk blocksize

typedef struct dfs_block {
  char data[DFS_BLOCKSIZE];
} dfs_block;

#define DFS_INODE_MAX_SIZE 96
#define DFS_INODE_MAX_FILENAME                                            \
  96 - (sizeof(inuse_t) + sizeof(filesize_t) + 10 * sizeof(block_idx_t) + \
        sizeof(block_idx_t))
#define NUM_VIRTUAL_BLOCKS 10
typedef struct dfs_inode {
  // STUDENT: put inode structure internals here
  // IMPORTANT: sizeof(dfs_inode) MUST return 128 in order to fit in enough
  // inodes in the filesystem (and to make your life easier).  To do this,
  // adjust the maximumm length of the filename until the size of the overall
  // inode
  // is 128 bytes.
  inuse_t inuse;
  filesize_t size;
  char filename[DFS_INODE_MAX_FILENAME];
  block_idx_t virtual_blocks[NUM_VIRTUAL_BLOCKS];
  block_idx_t indirect_block;

} dfs_inode;

#define DFS_MAX_FILESYSTEM_SIZE 0x1000000  // 16MB

#define DFS_INODE_MAX_NUM 192
#define DFS_MAX_NUM_BLOCKS (DFS_MAX_FILESYSTEM_SIZE / DFS_BLOCKSIZE)
// ceil
#define DFS_FBV_MAX_NUM_WORDS ((DFS_MAX_NUM_BLOCKS + 31) / 32)
#define DFS_INODE_START_BLOCK 1
#define DFS_INODE_NUM_BLOCKS \
  (DFS_INODE_MAX_NUM* DFS_INODE_MAX_SIZE / DFS_BLOCKSIZE)
#define DFS_FBV_START_BLOCK (DFS_INODE_NUM_BLOCKS + DFS_INODE_START_BLOCK)
#define DFS_DATA_START_BLOCK \
  (DFS_FBV_START_BLOCK + (DFS_FBV_MAX_NUM_WORDS * 4 / DFS_BLOCKSIZE))
#define DFS_FAIL -1
#define DFS_SUCCESS 1

#endif
