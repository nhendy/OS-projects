#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "disk.h"
#include "dfs.h"

void RunOSTests() {
  // STUDENT: run any os-level tests here
  //  int inode_f;
  //  char str1[] = "COVID";
  //  char str2[] = "2020";
  //  char str3[] = "Purdue";
  //  char str4[] = "OS";
  //  char indirect_addr[] = "Graduation";
  //  int f_address1 = 100;
  //  int f_address2 = 200;
  //  int f_address3 = 300;
  //  int f_address4 = 400;
  //  int indirect_addr = 50000;
  // char buffer[512];
  // printf("Starting RunOSTests function.\n");

  // if(DfsInodeFilenameExists("State") == DFS_FAIL)
  // {
  //   printf("\n file named State not found. \n File named State is now created.");
  //   inode_f = DfsInodeOpen("State");
    
    
  //   DfsInodeWriteBytes(inode_f, str1, f_address1, dstrlen(str1) + 1);
  //   printf("at file address %d string %s is written in file %s and size of inode file is %d\n", f_address1, str1, "State", DfsInodeFilesize(inode_f));

  //   DfsInodeWriteBytes(inode_f, str2, f_address2, dstrlen(str2) + 1);
  //   printf("at file address %d string %s is written in file %s and size of inode file is %d\n", f_address2, str2, "State", DfsInodeFilesize(inode_f));

  //   DfsInodeWriteBytes(inode_f, str3, f_address3, dstrlen(str3) + 1);
  //   printf("at file address %d string %s is written in file %s and size of inode file is %d\n", f_address3, str3, "State", DfsInodeFilesize(inode_f));

  //   DfsInodeWriteBytes(inode_f, str4, f_address4, dstrlen(str4) + 1);
  //   printf("Indirect Address Testing: At file address %d string %s is written in file %s and size of inode file is %d\n", indirect_addr, indirect_addr, "State", DfsInodeFilesize(inode_f));

  //   printf("Passed all test cases!");
  // }
  // else
  // {
  //   printf("\n file named State found.\n");
  //   inode_f = DfsInodeOpen("State");
    
  //   printf("Reading bytes @ %d\n", f_address1);
  //   DfsInodeReadBytes(inode_f, buffer, f_address1, dstrlen(str1) + 1);
  //   printf("Testing String %s : %s, Bytes Read: %s\n", str1, buffer);

  //   printf("Reading bytes @ %d\n", f_address2);
  //   DfsInodeReadBytes(inode_f, buffer, f_address2, dstrlen(str2) + 1);
  //   printf("Testing String %s : %s, Bytes Read: %s\n", str2, buffer);

  //   printf("Testing non-block-aligned sets of bytes\n", str2, buffer); 
  //   DfsInodeWriteBytes(inode_f, str4, f_address1, dstrlen(str4) + 1);
  //   printf("Size of inode file is %d\n ", DfsInodeFilesize(inode_f));
    
    
  //   printf("Reading bytes @ %d\n", f_address2);
  //   DfsInodeReadBytes(inode_f, buffer, f_address2, dstrlen(str2) + 1);
  //   printf("Testing String %s : %s, Bytes Read: %s\n", str2, buffer);

  //   printf("Reading bytes @ %d\n", f_address1);
  //   DfsInodeReadBytes(inode_f, buffer, f_address1, dstrlen(str1) + 1);
  //   printf("Testing String %s : %s, Bytes Read: %s\n", str1, buffer);// should be string 4
    
  //   printf("Reading bytes @ %d\n", indirect_addr);
  //   DfsInodeReadBytes(inode_f, buffer, indirect_addr, dstrlen(str2) + 1);
  //   printf("Indirect Addressing Test: Testing String %s : %s, Bytes Read: %s\n", indirect_addr, buffer);

  //   DfsInodeDelete(inode_f);
  //   printf("Passed all test cases!");
  //}
}


