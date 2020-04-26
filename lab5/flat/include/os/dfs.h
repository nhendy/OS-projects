#ifndef __DFS_H__
#define __DFS_H__

#include "dfs_shared.h"

uint32 DfsInodeOpen(char *filename);
uint32 DfsAllocateBlock();
int DfsCloseFileSystem();
int DfsFreeBlock(uint32 blocknum);
uint32 DfsInodeAllocateVirtualBlock(uint32 handle, uint32 virtual_blocknum);
int DfsInodeDelete(uint32 handle);
uint32 DfsInodeFilenameExists(char *filename);
uint32 DfsInodeFilesize(uint32 handle);
int DfsInodeIsValid(handle);
int DfsInodeReadBytes(uint32 handle, void *mem, int start_byte, int num_bytes);
uint32 DfsInodeTranslateVirtualToFilesys(uint32 handle,
                                         uint32 virtual_blocknum);
int DfsInodeWriteBytes(uint32 handle, void *mem, int start_byte, int num_bytes);
void DfsInvalidate();
int DfsIsValid();
void DfsModuleInit();
int DfsOpenFileSystem();
int DfsOpenFileSystem();
int DfsWriteBlock(uint32 blocknum, dfs_block *b);
void DumpSuperBlock();
int IsBlockAllocated(int block);
#endif
