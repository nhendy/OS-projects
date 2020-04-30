#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "disk.h"
#include "dfs.h"

#define BUFFSIZE 2048

typedef void (*function_ptr_t)();
function_ptr_t tests[100];
int num_tests = 0;
int num_failures = 0;
int num_checks = 0;

#define EXPECT_TRUE(val, message_format, args...)             \
  num_checks++;                                               \
  if (val) {                                                  \
    printf(KGRN "[%s|%d]: SUCCESS ", __FUNCTION__, __LINE__); \
  } else {                                                    \
    printf(KRED "[%s|%d]: FAIL: ", __FUNCTION__, __LINE__);   \
    num_failures++;                                           \
    printf(message_format, ##args);                           \
  }                                                           \
  printf("\n");                                               \
  printf(RESET)

#define EXPECT_TRUE_OR_FAIL(val, message_format, args...)         \
  num_checks++;                                                   \
  if (val) {                                                      \
    printf(KGRN "[%s|%d]: SUCCESS ", __FUNCTION__, __LINE__);     \
  } else {                                                        \
    printf(KRED "[%s|%d]: FATAL FAIL: ", __FUNCTION__, __LINE__); \
    num_failures++;                                               \
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
  printf(KCYN "\n\n Testing Allocate Block...\n\n" RESET);
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
  printf(KCYN "\n\n Testing Free Block...\n\n" RESET);
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
  printf(KCYN "\n\n Testing Inode File Exists...\n\n" RESET);
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
  printf(KCYN "\n\n Testing Inode Open...\n\n" RESET);
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
  printf(KCYN "\n\n Testing Inode FileSize...\n\n" RESET);
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
  printf(KCYN "\n\n Testing Scenario 1...\n\n" RESET);
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
  printf(KCYN "\n\n Testing Scenario 2...\n\n" RESET);
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

  EXPECT_TRUE(test_size == kStartByte + dstrlen(readbuff),
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
  // NOTE: need to delete inode here because subsequent tests will not rely on
  // previous ones
  EXPECT_TRUE(DfsInodeDelete(test_inode) != DFS_FAIL, "Failed to delete inode");
  LOG("ostests: INODE SCENARIO 2 succeeds: readbuff = %s, size of %s = %d\n",
      readbuff, kNonEmptyFile, test_size);
}

void TestReadWriteScenarioThree() {
  char readbuff[BUFFSIZE];
  uint32 test_inode = DfsInodeOpen(kNonEmptyFile);
  uint32 test_size;
  const uint32 kStartByte = DFS_BLOCKSIZE * 3 + 277;
  LOG("ostests: Starting INODE SCENARIO 3\n");
  printf(KCYN "\n\n Testing Scenario 3...\n\n" RESET);
  EXPECT_TRUE_OR_FAIL(test_inode != DFS_FAIL, "Failed to open file %s",
                      kNonEmptyFile);
  EXPECT_TRUE_OR_FAIL(DfsInodeFilesize(test_inode) == 0,
                      "File should be empty");

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

  EXPECT_TRUE(test_size == kStartByte + dstrlen(readbuff),
              "File size not update correctly");

  // NOTE: need to delete inode here because subsequent tests will not rely on
  // previous ones
  EXPECT_TRUE(DfsInodeDelete(test_inode) != DFS_FAIL, "Failed to delete inode");
  LOG("ostests: INODE SCENARIO 3 succeeds: readbuff = %s, size of %s = %d\n",
      readbuff, kNonEmptyFile, test_size);
}

void TestReadWriteScenarioFour() {
  char readbuff[BUFFSIZE];
  uint32 test_inode = DfsInodeOpen(kNonEmptyFile);
  uint32 test_size;
  const uint32 kStartByte = DFS_BLOCKSIZE - 4;
  LOG("ostests: Starting INODE SCENARIO 4\n");
  printf(KCYN "\n\n Testing Scenario 4...\n\n" RESET);
  EXPECT_TRUE_OR_FAIL(test_inode != DFS_FAIL, "Failed to open file %s",
                      kNonEmptyFile);
  EXPECT_TRUE_OR_FAIL(DfsInodeFilesize(test_inode) == 0,
                      "File should be empty");

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

  EXPECT_TRUE(test_size == kStartByte + dstrlen(readbuff),
              "File size not update correctly");

  // NOTE: need to delete inode here because subsequent tests will not rely on
  // previous ones
  EXPECT_TRUE(DfsInodeDelete(test_inode) != DFS_FAIL, "Failed to delete inode");
  LOG("ostests: INODE SCENARIO 4 succeeds: readbuff = %s, size of %s = %d\n",
      readbuff, kNonEmptyFile, test_size);
}

void TestReadWriteScenarioFive() {
  char readbuff[BUFFSIZE];
  uint32 test_inode = DfsInodeOpen(kNonEmptyFile);
  uint32 test_size;
  const uint32 kStartByte = DFS_BLOCKSIZE * 14;
  printf(KCYN "\n\n Testing Scenario 5...\n\n" RESET);
  LOG("ostests: Starting INODE SCENARIO 5\n");
  EXPECT_TRUE_OR_FAIL(test_inode != DFS_FAIL, "Failed to open file %s",
                      kNonEmptyFile);
  EXPECT_TRUE_OR_FAIL(DfsInodeFilesize(test_inode) == 0,
                      "File should be empty");

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

  EXPECT_TRUE(test_size == kStartByte + dstrlen(readbuff),
              "File size not update correctly");

  // NOTE: need to delete inode here because subsequent tests will not rely on
  // previous ones
  EXPECT_TRUE(DfsInodeDelete(test_inode) != DFS_FAIL, "Failed to delete inode");
  LOG("ostests: INODE SCENARIO 5 succeeds: readbuff = %s, size of %s = %d\n",
      readbuff, kNonEmptyFile, test_size);
}

void TestReadWriteScenarioSix() {
  char readbuff[BUFFSIZE];
  uint32 test_inode = DfsInodeOpen(kNonEmptyFile);
  uint32 test_size;
  const uint32 kStartByte = DFS_BLOCKSIZE * 9 + DFS_BLOCKSIZE - 4;
  LOG("ostests: Starting INODE SCENARIO 6\n");
  printf(KCYN "\n\n Testing Scenario 6...\n\n" RESET);
  EXPECT_TRUE_OR_FAIL(test_inode != DFS_FAIL, "Failed to open file %s",
                      kNonEmptyFile);
  EXPECT_TRUE_OR_FAIL(DfsInodeFilesize(test_inode) == 0,
                      "File should be empty");

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

  EXPECT_TRUE(test_size == kStartByte + dstrlen(readbuff),
              "File size not update correctly");

  // NOTE: need to delete inode here because subsequent tests will not rely on
  // previous ones
  EXPECT_TRUE(DfsInodeDelete(test_inode) != DFS_FAIL, "Failed to delete inode");
  LOG("ostests: INODE SCENARIO 6 succeeds: readbuff = %s, size of %s = %d\n",
      readbuff, kNonEmptyFile, test_size);
}

void TestReadWriteScenarioSeven() {
  char readbuff[BUFFSIZE], writebuff[BUFFSIZE];
  uint32 test_inode = DfsInodeOpen(kNonEmptyFile);
  uint32 test_size;
  const uint32 kStartByte = 0;
  printf(KCYN "\n\n Testing Scenario 7...\n\n" RESET);
  dstrncpy(writebuff, "Lorem ipsum dolor sit amet, consectetur adipiscing
  elit. Morbi vestibulum, massa nec porta blandit, nibh turpis faucibus
  sapien, nec finibus eros nibh a massa. Aliquam sagittis ligula eu ipsum
  dictum volutpat. Curabitur quis est nec purus sollicitudin sollicitudin
  consequat ac ex. Suspendisse urna urna, viverra et magna in, pharetra
  venenatis quam. Phasellus purus est, pharetra quis lacinia at, semper ac
  ipsum. Aliquam pharetra et tellus et lacinia. Phasellus elit ante,
  egestas
  et elementum ac, malesuada eu magna. In viverra libero sed ipsum
  interdum,
  vel dictum nibh condimentum. Duis sit amet ligula non leo pharetra
  scelerisque. Etiam nisl magna, ultricies vitae pulvinar ac, interdum
  mollis
  lectus. Curabitur porta sem aliquet sagittis dignissim. Fusce sapien
  purus,
  convallis dignissim placerat nec, hendrerit ac nibh. Nunc eleifend
  euismod
  convallis. Aliquam sed nisi suscipit, vulputate nunc eget, mollis
  turpis."
                      "Ut ornare ligula ac quam convallis pharetra.
  Praesent
  fermentum, dui ut posuere fringilla, augue eros euismod arcu, non
  sollicitudin eros magna at ante. Integer id sapien tempus, dictum nibh
  vitae, finibus sem. In non venenatis nisi. Donec eu nisl dictum, vehicula
  nunc nec, porttitor ante. Integer consectetur ut neque vitae aliquam.
  Duis
  at convallis dui."
                      "Mauris viverra sed mi quis vehicula. Nullam augue
  augue, mollis quis tempus sodales, molestie quis mi. Nam ac risus sed
  nibh
  faucibus commodo. Sed sollicitudin augue at venenatis tristique. Integer
  volutpat eros vitae velit lobortis rutrum. Aenean metus.",
                      1536);
  LOG("ostests: Starting INODE SCENARIO 7\n");
  EXPECT_TRUE_OR_FAIL(test_inode != DFS_FAIL, "Failed to open file %s",
                      kNonEmptyFile);
  EXPECT_TRUE_OR_FAIL(DfsInodeFilesize(test_inode) == 0,
                      "File should be empty");

  // Attempts to write to a file at the start of the file
  EXPECT_TRUE(DfsInodeWriteBytes(test_inode, writebuff, kStartByte,
                                 dstrlen(writebuff)) == dstrlen(writebuff),
              "Wasn't able to write as many bytes as expected");

  // Attempts to read from that same file at the start of the file
  EXPECT_TRUE(DfsInodeReadBytes(test_inode, readbuff, kStartByte,
                                dstrlen(writebuff)) == dstrlen(writebuff),
              "Wasn't able to read as many bytes as expected");

  EXPECT_TRUE(dstrncmp(readbuff, writebuff, dstrlen(writebuff)) == 0,
              "Whatever was read is different");
  test_size = DfsInodeFilesize(test_inode);

  EXPECT_TRUE(test_size == kStartByte + dstrlen(readbuff),
              "File size not update correctly");

  // NOTE: need to delete inode here because subsequent tests will not rely on
  // previous ones
  EXPECT_TRUE(DfsInodeDelete(test_inode) != DFS_FAIL, "Failed to delete inode");
  LOG("ostests: INODE SCENARIO 7 succeeds: readbuff = %s, size of %s = %d\n",
      readbuff, kNonEmptyFile, test_size);
}

void TestOpenDelete() {
  uint32 test_inode = DfsInodeOpen(kNonEmptyFile);
  printf(KCYN "\n\n Testing Inode Open and Delete...\n\n" RESET);
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
  // Requested scenarios
  REGISTER_TEST(TestReadWriteScenarioOne);
  REGISTER_TEST(TestReadWriteScenarioTwo);
  REGISTER_TEST(TestReadWriteScenarioThree);
  REGISTER_TEST(TestReadWriteScenarioFour);
  REGISTER_TEST(TestReadWriteScenarioFive);
  REGISTER_TEST(TestReadWriteScenarioSix);
  REGISTER_TEST(TestReadWriteScenarioSeven);
}

void runAllTests() {
  int i;
  for (i = 0; i < num_tests; ++i) {
    tests[i]();
  }
}

void RunOSTests() {
  setupTests();
  LOG("Num tests: %d\n", num_tests);
  runAllTests();

  printf(KGRN
         "\n\n================================================================="
         "=================================\n");
  printf("Os tests:                    %d succeeded, %d failed \n",
         num_checks - num_failures, num_failures);
  printf(
      "========================================================================"
      "==========================\n\n" RESET);
  return;
}
