#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "disk.h"
#include "dfs.h"

#define BUFFSIZE 2048
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define RESET "\x1B[0m"

typedef void (*function_ptr_t)();
function_ptr_t tests[100];
int num_tests = 0;

#define EXPECT_TRUE(val, message_format, args...)             \
  if (val) {                                                  \
    printf(KGRN "[%s|%d]: SUCCESS ", __FUNCTION__, __LINE__); \
  } else {                                                    \
    printf(KRED "[%s|%d]: FAIL: ", __FUNCTION__, __LINE__);   \
    printf(message_format, ##args);                           \
  }                                                           \
  printf("\n");                                               \
  printf(RESET)

#define EXPECT_TRUE_OR_FAIL(val, message_format, args...)         \
  if (val) {                                                      \
    printf(KGRN "[%s|%d]: SUCCESS ", __FUNCTION__, __LINE__);     \
  } else {                                                        \
    printf(KRED "[%s|%d]: FATAL FAIL: ", __FUNCTION__, __LINE__); \
    printf(message_format, ##args);                               \
    printf("\n");                                                 \
    printf(RESET);                                                \
    return;                                                       \
  }                                                               \
  printf("\n");                                                   \
  printf(RESET)

#define REGISTER_TEST(function) tests[num_tests++] = &function

const char* kNonExistenFileOne = "nonexistent.txt";
const char* kNonExistenFileTwo = "nonexistent_2.txt";
const char* kEmptyFile = "empty.txt";
const char* kNonEmptyFile = "nonempty.txt";
const char* kSmallString = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";

void TestAllocateAllBlocks() {
  int i;
  LOG("Allocating 16500 blocks\n");
  for (i = 0; i < 16500; i++) {
    LOG("RunOSTests: Allocating block %d\n", i);
    if (i < DFS_MAX_NUM_BLOCKS) {
      EXPECT_TRUE(
          DfsAllocateBlock() != DFS_FAIL,
          "Failed to allocated block %d, maybe need to reformat the disk?\n");

    } else {
      EXPECT_TRUE(DfsAllocateBlock() == DFS_FAIL,
                  "Shouldn't be able to allocate block %d. Sth is wrong!!\n");
    }
  }
}

void TestFreeBlocks() {

  uint32 test_block;
  printf(KGRN "---------------------------------%s\n" RESET, __FUNCTION__);
  test_block = DfsAllocateBlock();
  EXPECT_TRUE_OR_FAIL(test_block != DFS_FAIL,
                      "Failed to allocate a block. Maybe reformat disk?");

  EXPECT_TRUE(
      DfsFreeBlock(0) != DFS_FAIL,
      "Block 0 (metadata/superblock) should be allocated. Sth is wrong");

  EXPECT_TRUE(DfsFreeBlock(test_block) != DFS_FAIL, "Failed to free block %d",
              test_block);
  EXPECT_TRUE(DfsFreeBlock(test_block) == DFS_FAIL,
              "Woaah!, double free'ed a block!!");

  EXPECT_TRUE(DfsFreeBlock(DFS_MAX_NUM_BLOCKS + 1),
              "Wooah! free'ed block outside FS");
}

void TestInodeFileExists() {
  EXPECT_TRUE(DfsInodeFilenameExists(kNonExistenFileOne) == DFS_FAIL,
              "%s should not exist", kNonExistenFileOne);
  EXPECT_TRUE(DfsInodeFilenameExists(kNonExistenFileTwo) == DFS_FAIL,
              "%s should not exist", kNonExistenFileTwo);
  EXPECT_TRUE(DfsInodeFilenameExists(
                  "areallylongfilenamethisistolongtobeinaninodesothereshouldbea"
                  "nerror.txt") == DFS_FAIL,
              "Long file name should not exist");
}

void TestInodeOpen() {
  EXPECT_TRUE(DfsInodeOpen("heep.txt") != DFS_FAIL, "Failed to open heep.txt");
  EXPECT_TRUE(DfsInodeOpen(kEmptyFile) != DFS_FAIL, "Failed to open %s",
              kEmptyFile);
  EXPECT_TRUE(DfsInodeOpen(kEmptyFile) != DFS_FAIL, "Double open should work");
  EXPECT_TRUE(DfsInodeOpen(
                  "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                  "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                  "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.txt") == DFS_FAIL,
              "Shouldn't be able to open file with long name");
}

void TestInodeFileSize() {
  uint32 test_inode;
  test_inode = DfsInodeFilenameExists(kEmptyFile);
  EXPECT_TRUE_OR_FAIL(test_inode != DFS_FAIL, "Failed to find file %s",
                      kEmptyFile);
  EXPECT_TRUE(DfsInodeFilesize(test_inode) == 0, "File %s should be empty",
              kEmptyFile);
  EXPECT_TRUE(DfsInodeFilesize(-1) == DFS_FAIL,
              "Woaah! passed a negative handle");
  EXPECT_TRUE(DfsInodeFilesize(DFS_INODE_MAX_NUM + 1) == DFS_FAIL,
              "Woaah! passed a over bounds inode handle");
  EXPECT_TRUE(DfsInodeFilesize(0) != DFS_FAIL,
              "inode 0 should have a size if there is at least one file");
}

void TestReadWriteScenarioOne() {
  char readbuff[BUFFSIZE];
  uint32 test_inode = DfsInodeOpen(kNonEmptyFile);
  uint32 test_size;
  LOG("ostests: Starting INODE SCENARIO 1\n");
  EXPECT_TRUE_OR_FAIL(test_inode != DFS_FAIL, "Failed to open file %s",
                      kNonEmptyFile);
  EXPECT_TRUE_OR_FAIL(DfsInodeFilesize(test_inode) == 0,
                      "File should be empty");

  // Attempts to write to a file at the start of the file
  EXPECT_TRUE(
      DfsInodeWriteBytes(test_inode, kSmallString, 0, dstrlen(kSmallString)) ==
          dstrlen(kSmallString),
      "Wasn't able to write as many bytes as expected");

  // Attempts to read from that same file at the start of the file
  EXPECT_TRUE(DfsInodeReadBytes(test_inode, readbuff, 0,
                                dstrlen(kSmallString)) == dstrlen(kSmallString),
              "Wasn't able to read as many bytes as expected");

  EXPECT_TRUE(dstrncmp(readbuff, kSmallString, dstrlen(kSmallString)) == 0,
              "Whatever was read is different");
  test_size = DfsInodeFilesize(test_inode);

  EXPECT_TRUE(test_size == dstrlen(readbuff), "File size not update correctly");

  LOG("ostests: INODE SCENARIO 1 succeeds: readbuff = %s, size of %s = %d\n",
      readbuff, kNonEmptyFile, test_size);
}

void TestReadWriteScenarioTwo() {
  char readbuff[BUFFSIZE];
  uint32 test_inode = DfsInodeOpen(kNonEmptyFile);
  uint32 test_size;
  const uint32 kStartByte = DFS_BLOCKSIZE * 2;
  LOG("ostests: Starting INODE SCENARIO 1\n");
  EXPECT_TRUE_OR_FAIL(test_inode != DFS_FAIL, "Failed to open file %s",
                      kNonEmptyFile);
  EXPECT_TRUE_OR_FAIL(DfsInodeFilesize(test_inode) == dstrlen(kSmallString),
                      "File should be of small string size");

  // Attempts to write to a file at the start of the file
  EXPECT_TRUE(
      DfsInodeWriteBytes(test_inode, kSmallString, kStartByte,
                         dstrlen(kSmallString)) == dstrlen(kSmallString),
      "Wasn't able to write as many bytes as expected");

  // Attempts to read from that same file at the start of the file
  EXPECT_TRUE(DfsInodeReadBytes(test_inode, readbuff, kStartByte,
                                dstrlen(kSmallString)) == dstrlen(kSmallString),
              "Wasn't able to read as many bytes as expected");

  EXPECT_TRUE(dstrncmp(readbuff, kSmallString, dstrlen(kSmallString)) == 0,
              "Whatever was read is different");
  test_size = DfsInodeFilesize(test_inode);

  EXPECT_TRUE(test_size == dstrlen(readbuff) * 2,
              "File size not update correctly");

  // Read what was written in scenario #1
  EXPECT_TRUE(DfsInodeReadBytes(test_inode, readbuff, 0,
                                dstrlen(kSmallString)) == dstrlen(kSmallString),
              "Wasn't able to read as many bytes as expected");

  EXPECT_TRUE(dstrncmp(readbuff, kSmallString, dstrlen(kSmallString)) == 0,
              "Whatever was read is different");

  // Write to another non contiguous byte
  EXPECT_TRUE(
      DfsInodeWriteBytes(test_inode, kSmallString, kStartByte / 2,
                         dstrlen(kSmallString)) == dstrlen(kSmallString),
      "Wasn't able to write as many bytes as expected");

  // Attempts to read from that same file at the start of the file
  EXPECT_TRUE(DfsInodeReadBytes(test_inode, readbuff, kStartByte / 2,
                                dstrlen(kSmallString)) == dstrlen(kSmallString),
              "Wasn't able to read as many bytes as expected");

  LOG("ostests: INODE SCENARIO 2 succeeds: readbuff = %s, size of %s = %d\n",
      readbuff, kNonEmptyFile, test_size);
}

void TestOpenDelete() {
  uint32 test_inode = DfsInodeOpen(kNonEmptyFile);
  EXPECT_TRUE_OR_FAIL(test_inode != DFS_FAIL, "Failed to open file %s",
                      kNonEmptyFile);
  EXPECT_TRUE(DfsInodeDelete(test_inode) != DFS_FAIL, "Failed to delete inode");
}

void setupTests() {
  // REGISTER_TEST(&TestAllocateAllBlocks);
  // REGISTER_TEST(&TestFreeBlocks);
  // NOTE: Run this test first since it sets up files for subsequence tests
  REGISTER_TEST(TestOpenDelete);
  REGISTER_TEST(TestInodeFileExists);
  REGISTER_TEST(TestInodeOpen);
  REGISTER_TEST(TestInodeFileSize);
  REGISTER_TEST(TestReadWriteScenarioOne);
  REGISTER_TEST(TestReadWriteScenarioTwo);
}

void runAllTests() {
  int i;
  for (i = 0; i < num_tests; ++i) {
    tests[i]();
  }
}

void RunOSTests() {
  // STUDENT: run any os-level tests here

  // FSDRIVER BASED TESTS

  /* INODE BASED TESTS */
  // Checks some existences
  setupTests();
  LOG("Num tests: %d\n", num_tests);
  runAllTests();

  /******** INODE SCENARIO 0: Tests Opening inodes, including ones with
   * filenames that are too long ********/

  /******** INODE SCENARIO 1: Attempts writing and reading a simple string
  from the first byte position of an inode, as well as including file size
  verification ********/

  // /******** INODE SCENARIO 2: Attempts writing non-contiguous parts of file
  // and verifying maxbyte ********/
  // // Test plan:
  // //  1) Write into the file we wrote into in scenario 1. This write will
  // go
  // into a different block than the first write,
  // //     but will still be aligned to byte 0 of the block
  // //  2) Read this string back and verify that it is what we believe it to
  // be
  // //  3) Read back the original write from scenario 1 to make sure it is
  // still fine
  // //  4) Read the file size.
  // //  4) Write to a location in between the two writes from scenario 1 and
  // 2
  // to make sure that the filesize stays the same
  // //  5) Read the file size. Even though we have only written a few bytes,
  // the maxbyte should still be the end of the write in step 1

  // bzero(readbuff, BUFFSIZE);
  // printf("ostests: Starting INODE SCENARIO 2\n");
  // // Attempts to write to file at the start of a different block
  // // This scenario assumes a DFS_BLOCKSIZE blocksize
  // if (DfsInodeWriteBytes(handleMeep, writebuff, DFS_BLOCKSIZE * 2,
  //                        dstrlen(writebuff)) != dstrlen(writebuff)) {
  //   printf("ostests: INODE SCENARIO 2 - failed to write %s to inode %u at
  // position %d\n", writebuff, handleMeep, DFS_BLOCKSIZE * 2);
  //   GracefulExit();
  // }

  // if (DfsInodeReadBytes(handleMeep, readbuff, DFS_BLOCKSIZE * 2,
  //                       dstrlen(writebuff)) != dstrlen(writebuff)) {
  //   printf("ostests: INODE SCENARIO 2 - failed to read from inode %u at
  // position %d\n", handleMeep, DFS_BLOCKSIZE * 2);
  //   GracefulExit();
  // }

  // if (dstrncmp(readbuff, writebuff, dstrlen(writebuff)) != 0) {
  //   printf("ostests: INODE SCENARIO 2 - did not read data correctly from
  // inode %u\n, actual = %s\n", handleMeep, readbuff);
  //   GracefulExit();
  // }

  // bzero(readbuff, BUFFSIZE);

  // if (DfsInodeReadBytes(handleMeep, readbuff, 0, dstrlen(writebuff)) !=
  //     dstrlen(writebuff)) {
  //   printf("ostests: INODE SCENARIO 2 - failed to read from inode %u at
  // position %d\n", handleMeep, 0);
  //   GracefulExit();
  // }

  // if (dstrncmp(readbuff, writebuff, dstrlen(writebuff)) != 0) {
  //   printf("ostests: INODE SCENARIO 2 - did not read data correctly from
  // inode %u\n, actual = %s\n", handleMeep, readbuff);
  //   GracefulExit();
  // }

  // // This scenario assumes that this is writing into the last position in
  // // the file, and therefore the filesize of the inode
  // // should be the base + num_bytes of this write
  // if (DfsInodeFilesize(handleMeep) !=
  //     ((DFS_BLOCKSIZE * 2) + dstrlen(writebuff))) {
  //   printf("ostests: INODE SCENARIO 2 - handle %u size is %d, should be
  // %d!\n", handleMeep, DfsInodeFilesize(handleMeep), ((DFS_BLOCKSIZE * 2) +
  // dstrlen(writebuff)));
  //   GracefulExit();
  // }

  // if (DfsInodeWriteBytes(handleMeep, writebuff, DFS_BLOCKSIZE * 1,
  //                        dstrlen(writebuff)) != dstrlen(writebuff)) {
  //   printf("ostests: INODE SCENARIO 2 - failed to write %s to inode %u at
  // position %d\n", writebuff, handleMeep, DFS_BLOCKSIZE * 1);
  //   GracefulExit();
  // }

  // if (DfsInodeFilesize(handleMeep) !=
  //     ((DFS_BLOCKSIZE * 2) + dstrlen(writebuff))) {
  //   printf("ostests: INODE SCENARIO 2 - handle %u size is %d, should be
  // %d!\n", handleMeep, DfsInodeFilesize(handleMeep), ((DFS_BLOCKSIZE * 2) +
  // dstrlen(writebuff)));
  //   GracefulExit();
  // }

  // printf("ostests: INODE SCENARIO 2 succeeds: readbuff = %s, size of
  // meep.txt
  // = %d\n", readbuff, DfsInodeFilesize(handleMeep));

  // printf("\n\n");

  // /******** INODE SCENARIO 3: Attempts writing non block-aligned parts of a
  // file ********/
  // // Test plan:
  // //  1) Write to a block that has already been allocated not at the
  // beginning of the block
  // //  2) Read from the block to make sure that the data is still the same
  // //  3) Write to a block that has not already been allocated on a
  // non-aligned space
  // //  4) Repeat step 2 for this block

  // bzero(readbuff, BUFFSIZE);
  // printf("ostests: Starting INODE SCENARIO 3\n");
  // // Attempts to write to file at the start of a different block
  // // This scenario assumes a DFS_BLOCKSIZE blocksize
  // if (DfsInodeWriteBytes(handleMeep, writebuff, 357, dstrlen(writebuff)) !=
  // dstrlen(writebuff)) {
  //   printf("ostests: INODE SCENARIO 3 - failed to write %s to inode %u at
  // position %d\n", writebuff, handleMeep, 357);
  //   GracefulExit();
  // }

  // if (DfsInodeReadBytes(handleMeep, readbuff, 357, dstrlen(writebuff)) !=
  // dstrlen(writebuff)) {
  //   printf("ostests: INODE SCENARIO 3 - failed to read from inode %u at
  // position %d\n", handleMeep, 357);
  //   GracefulExit();
  // }

  // if (dstrncmp(readbuff, writebuff, dstrlen(writebuff)) != 0) {
  //   printf("ostests: INODE SCENARIO 3 - did not read data correctly from
  // inode %u\n, actual = %s\n", handleMeep, readbuff);
  //   GracefulExit();
  // }

  // bzero(readbuff, BUFFSIZE);

  // if (DfsInodeWriteBytes(handleMeep, writebuff, (DFS_BLOCKSIZE * 3) + 357,
  // dstrlen(writebuff)) != dstrlen(writebuff)) {
  //   printf("ostests: INODE SCENARIO 3 - failed to write %s to inode %u at
  // position %d\n", writebuff, handleMeep, (DFS_BLOCKSIZE * 3) + 357);
  //   GracefulExit();
  // }

  // if (DfsInodeReadBytes(handleMeep, readbuff, 357, dstrlen(writebuff)) !=
  // dstrlen(writebuff)) {
  //   printf("ostests: INODE SCENARIO 3 - failed to read from inode %u at
  // position %d\n", handleMeep, (DFS_BLOCKSIZE * 3) + 357);
  //   GracefulExit();
  // }

  // if (dstrncmp(readbuff, writebuff, dstrlen(writebuff)) != 0) {
  //   printf("ostests: INODE SCENARIO 3 - did not read data correctly from
  // inode %u\n, actual = %s\n", handleMeep, readbuff);
  //   GracefulExit();
  // }

  // printf("ostests: INODE SCENARIO 3 succeeds: readbuff = %s, size of
  // meep.txt
  // = %d\n", readbuff, DfsInodeFilesize(handleMeep));

  // printf("\n\n");

  // /******** INODE SCENARIO 4: Attempts writing across block boundaries
  // ********/
  // // Test plan:
  // //  1) Write to a block such that the contents of the write will cross
  // over
  // //     into the next block
  // //  2) Read the data and verify that the data has been read correctly

  // bzero(readbuff, BUFFSIZE);
  // printf("ostests: Starting INODE SCENARIO 4\n");
  // // Attempts to write to file at the end of a block
  // // This scenario assumes a DFS_BLOCKSIZE blocksize
  // if (DfsInodeWriteBytes(handleMeep, writebuff, 1020, dstrlen(writebuff))
  // !=
  // dstrlen(writebuff)) {
  //   printf("ostests: INODE SCENARIO 4 - failed to write %s to inode %u at
  // position %d\n", writebuff, handleMeep, 1020);
  //   GracefulExit();
  // }

  // if (DfsInodeReadBytes(handleMeep, readbuff, 1020, dstrlen(writebuff)) !=
  // dstrlen(writebuff)) {
  //   printf("ostests: INODE SCENARIO 4 - failed to read from inode %u at
  // position %d\n", handleMeep, 1020);
  //   GracefulExit();
  // }

  // if (dstrncmp(readbuff, writebuff, dstrlen(writebuff)) != 0) {
  //   printf("ostests: INODE SCENARIO 4 - did not read data correctly from
  // inode %u\n, actual = %s\n", handleMeep, readbuff);
  //   GracefulExit();
  // }

  // printf("ostests: INODE SCENARIO 4 succeeds - readbuff = %s, size of
  // meep.txt = %d\n", readbuff, DfsInodeFilesize(handleMeep));

  // printf("\n\n");

  // /******** INODE SCENARIO 5: Attempts writing into the indirect addressing
  // space ********/
  // // Test plan:
  // //  1) Write to a block such that the contents of the write will be
  // indirectly addressed
  // //     by the inode
  // //  2) Read the data and verify that it has been written correctly

  // bzero(readbuff, BUFFSIZE);
  // printf("ostests: Starting INODE SCENARIO 5\n");
  // // This scenario assumes a DFS_BLOCKSIZE blocksize
  // if (DfsInodeWriteBytes(handleMeep, writebuff, (DFS_BLOCKSIZE * 11),
  // dstrlen(writebuff)) != dstrlen(writebuff)) {
  //   printf("ostests: INODE SCENARIO 5 - failed to write %s to inode %u at
  // position %d\n", writebuff, handleMeep, (DFS_BLOCKSIZE * 11));
  //   GracefulExit();
  // }

  // if (DfsInodeReadBytes(handleMeep, readbuff, (DFS_BLOCKSIZE * 11),
  // dstrlen(writebuff)) != dstrlen(writebuff)) {
  //   printf("ostests: INODE SCENARIO 5 - failed to read from inode %u at
  // position %d\n", handleMeep, (DFS_BLOCKSIZE * 11));
  //   GracefulExit();
  // }

  // if (dstrncmp(readbuff, writebuff, dstrlen(writebuff)) != 0) {
  //   printf("ostests: INODE SCENARIO 5 - did not read data correctly from
  // inode %u\n, actual = %s\n", handleMeep, readbuff);
  //   GracefulExit();
  // }

  // printf("ostests: INODE SCENARIO 5 succeeds - readbuff = %s, size of
  // meep.txt = %d\n", readbuff, DfsInodeFilesize(handleMeep));

  // printf("\n\n");

  // /******** INODE SCENARIO 6: Attempts writing into the end of the last
  // direct block, into the indirectly addressed space ********/
  // // Test plan: Repeat scenario 4, except start in the last direct block
  // and
  // write into the first indirect block

  // bzero(readbuff, BUFFSIZE);
  // printf("ostests: Starting INODE SCENARIO 6\n");
  // // Attempts to write to file at the end of a block
  // // This scenario assumes a DFS_BLOCKSIZE blocksize
  // if (DfsInodeWriteBytes(handleMeep, writebuff, (DFS_BLOCKSIZE * 9) + 1020,
  // dstrlen(writebuff)) != dstrlen(writebuff)) {
  //   printf("ostests: INODE SCENARIO 6 - failed to write %s to inode %u at
  // position %d\n", writebuff, handleMeep, (DFS_BLOCKSIZE * 9) + 1020);
  //   GracefulExit();
  // }

  // if (DfsInodeReadBytes(handleMeep, readbuff, (DFS_BLOCKSIZE * 9) + 1020,
  // dstrlen(writebuff)) != dstrlen(writebuff)) {
  //   printf("ostests: INODE SCENARIO 6 - failed to read from inode %u at
  // position %d\n", handleMeep, (DFS_BLOCKSIZE * 9) + 1020);
  //   GracefulExit();
  // }

  // if (dstrncmp(readbuff, writebuff, dstrlen(writebuff)) != 0) {
  //   printf("ostests: INODE SCENARIO 6 - did not read data correctly from
  // inode %u\n, actual = %s\n", handleMeep, readbuff);
  //   GracefulExit();
  // }

  // printf("ostests: INODE SCENARIO 6 succeeds - readbuff = %s, size of
  // meep.txt = %d\n", readbuff, DfsInodeFilesize(handleMeep));

  // printf("\n\n");

  // /******** INODE SCENARIO 7: Write more than a block of data ********/
  // // Test plan:
  // //  1) Write content that is larger than the block size
  // //  2) Try to read it back

  // bzero(readbuff, BUFFSIZE);
  // printf("ostests: Starting INODE SCENARIO 7\n");
  // // Creates the string

  // dstrncpy(writebuff, "Lorem ipsum dolor sit amet, consectetur adipiscing
  // elit. Morbi vestibulum, massa nec porta blandit, nibh turpis faucibus
  // sapien, nec finibus eros nibh a massa. Aliquam sagittis ligula eu ipsum
  // dictum volutpat. Curabitur quis est nec purus sollicitudin sollicitudin
  // consequat ac ex. Suspendisse urna urna, viverra et magna in, pharetra
  // venenatis quam. Phasellus purus est, pharetra quis lacinia at, semper ac
  // ipsum. Aliquam pharetra et tellus et lacinia. Phasellus elit ante,
  // egestas
  // et elementum ac, malesuada eu magna. In viverra libero sed ipsum
  // interdum,
  // vel dictum nibh condimentum. Duis sit amet ligula non leo pharetra
  // scelerisque. Etiam nisl magna, ultricies vitae pulvinar ac, interdum
  // mollis
  // lectus. Curabitur porta sem aliquet sagittis dignissim. Fusce sapien
  // purus,
  // convallis dignissim placerat nec, hendrerit ac nibh. Nunc eleifend
  // euismod
  // convallis. Aliquam sed nisi suscipit, vulputate nunc eget, mollis
  // turpis."
  //                     "Ut ornare ligula ac quam convallis pharetra.
  // Praesent
  // fermentum, dui ut posuere fringilla, augue eros euismod arcu, non
  // sollicitudin eros magna at ante. Integer id sapien tempus, dictum nibh
  // vitae, finibus sem. In non venenatis nisi. Donec eu nisl dictum, vehicula
  // nunc nec, porttitor ante. Integer consectetur ut neque vitae aliquam.
  // Duis
  // at convallis dui."
  //                     "Mauris viverra sed mi quis vehicula. Nullam augue
  // augue, mollis quis tempus sodales, molestie quis mi. Nam ac risus sed
  // nibh
  // faucibus commodo. Sed sollicitudin augue at venenatis tristique. Integer
  // volutpat eros vitae velit lobortis rutrum. Aenean metus.",
  //                     1536);

  // // This scenario assumes a DFS_BLOCKSIZE blocksize
  // if (DfsInodeWriteBytes(handleMeep, writebuff, 0, dstrlen(writebuff)) !=
  // dstrlen(writebuff)) {
  //   printf("ostests: INODE SCENARIO 7 - failed to write %s to inode %u at
  // position %d\n", writebuff, handleMeep, 0);
  //   GracefulExit();
  // }

  // if (DfsInodeReadBytes(handleMeep, readbuff, 0, dstrlen(writebuff)) !=
  // dstrlen(writebuff)) {
  //   printf("ostests: INODE SCENARIO 7 - failed to read from inode %u at
  // position %d\n", handleMeep, 0);
  //   GracefulExit();
  // }

  // if (dstrncmp(readbuff, writebuff, dstrlen(writebuff)) != 0) {
  //   printf("ostests: INODE SCENARIO 7 - did not read data correctly from
  // inode %u\n, actual = %s\n", handleMeep, readbuff);
  //   GracefulExit();
  // }

  // printf("ostests: INODE SCENARIO 7 succeeds - readbuff = %s, size of
  // meep.txt = %d\n", readbuff, DfsInodeFilesize(handleMeep));

  // printf("\n\n");

  // // Attempts to delete a couple of the files and then open one again
  // if (DfsInodeDelete(handleMeep) != DFS_SUCCESS) {
  //   printf("ostests: deleting inode %u (meep.txt) failed\n", handleMeep);
  //   GracefulExit();
  // }

  // if (DfsInodeDelete(handleMarp) != DFS_SUCCESS) {
  //   printf("ostests: deleting inode %u (marp.txt) failed\n", handleMarp);
  //   GracefulExit();
  // }

  // // This should fail because the inode handle is DFS_FAIL
  // if (DfsInodeDelete(handleLong) != DFS_FAIL) {
  //   printf("ostests: deleting inode %u succeeded\n", handleLong);
  //   GracefulExit();
  // }

  // // This should also fail because the inode has already been freed
  // if (DfsInodeDelete(handleMeep) != DFS_FAIL) {
  //   printf("ostests: deleting inode %u (meep.txt - ALREADY DELETED)
  // succeeded\n", handleMeep);
  //   GracefulExit();
  // }

  return;
}
