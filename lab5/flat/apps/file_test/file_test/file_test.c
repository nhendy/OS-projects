#include "usertraps.h"
#include "files_shared.h"

#define KRED "\x1B[31m"
#define KCYN "\x1B[36m"
#define KGRN "\x1B[32m"
#define RESET "\x1B[0m"

typedef void (*function_ptr_t)();
function_ptr_t tests[100];
int num_tests = 0;
int num_failures = 0;

#define EXPECT_TRUE(val, message_format, args...)             \
  if (val) {                                                  \
    Printf(KGRN "[%s|%d]: SUCCESS ", __FUNCTION__, __LINE__); \
  } else {                                                    \
    Printf(KRED "[%s|%d]: FAIL: ", __FUNCTION__, __LINE__);   \
    num_failures++;                                           \
    Printf(message_format, ##args);                           \
  }                                                           \
  Printf("\n" RESET)

#define EXPECT_TRUE_OR_FAIL(val, message_format, args...)         \
  if (val) {                                                      \
    Printf(KGRN "[%s|%d]: SUCCESS ", __FUNCTION__, __LINE__);     \
  } else {                                                        \
    Printf(KRED "[%s|%d]: FATAL FAIL: ", __FUNCTION__, __LINE__); \
    Printf(message_format, ##args);                               \
    num_failures++;                                               \
    Printf("\n");                                                 \
    Printf(RESET);                                                \
    return;                                                       \
  }                                                               \
  Printf("\n" RESET)

#define REGISTER_TEST(function) tests[num_tests++] = &function
const char *kTestString =
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";

void TestFileOpen() {
  const char *kDummyFile = "dummy.txt";
  const int kMaxNumOpenFiles = 15;
  char *random_file_names[20] = {
      "0.txt",  "1.txt",  "2.txt",  "3.txt",  "4.txt",  "5.txt",  "6.txt",
      "7.txt",  "8.txt",  "9.txt",  "10.txt", "11.txt", "12.txt", "13.txt",
      "14.txt", "15.txt", "16.txt", "17.txt", "18.txt", "19.txt"};
  int i = 0;
  Printf(KCYN "\n\n Testing FileOpen...\n\n" RESET);
  EXPECT_TRUE(file_open(kDummyFile, "x") == FILE_FAIL, "Woah ! Invalid mode");
  EXPECT_TRUE(file_open(kDummyFile, "rx") == FILE_FAIL, "Woah ! Invalid mode");
  EXPECT_TRUE(file_open(kDummyFile, "ddd") == FILE_FAIL, "Woah ! Invalid mode");
  EXPECT_TRUE(file_open(kDummyFile, "rw") != FILE_FAIL,
              "Failed to open in RW mode");
  EXPECT_TRUE(file_open(kDummyFile, "rw") == FILE_FAIL,
              "Woah! should not be able to double open");
  EXPECT_TRUE(file_open(kDummyFile, "r") == FILE_FAIL,
              "Woah! should not be able to double open in R mode");
  EXPECT_TRUE(file_open(kDummyFile, "w") == FILE_FAIL,
              "Woah! should not be able to double open in W mode");
  for (i = 0; i < 20; ++i) {
    if (i < kMaxNumOpenFiles - 1) {
      EXPECT_TRUE(file_open(random_file_names[i], "r") != FILE_FAIL,
                  "Failed to open file %d", i);
    } else {
      EXPECT_TRUE(file_open(random_file_names[i], "r") == FILE_FAIL,
                  "Woah! opening file %d more than maximum", i);
    }
  }
  for (i = 0; i < 20; ++i) {
    if (i < kMaxNumOpenFiles - 1) {
      EXPECT_TRUE(file_delete(random_file_names[i]) != FILE_FAIL,
                  "Failed to delete file %d", i);
    }
  }
}

void TestFileClose() {
  const char *kTestFile = "testclosefile.txt";
  unsigned int fd;
  Printf(KCYN "\n\n Testing FileClose...\n\n" RESET);
  EXPECT_TRUE(file_close(9999) == FILE_FAIL,
              "Woah! should not be able to close invalid handle");
  EXPECT_TRUE(file_close(-1) == FILE_FAIL,
              "Woah! should not be able to close invalid handle");
  EXPECT_TRUE((fd = file_open(kTestFile, "w")) != FILE_FAIL,
              "Failed to open file");
  EXPECT_TRUE(file_close(fd) == FILE_SUCCESS, "Failed to close file");
  EXPECT_TRUE(file_close(fd) == FILE_FAIL, "Woaah! can't double close a file");
  EXPECT_TRUE(file_delete(kTestFile) == FILE_SUCCESS, "Failed to delete file");
}

void TestFileWrite() {
  const char *kTestFile = "testwritefile.txt";
  unsigned int fd;
  Printf(KCYN "\n\n Testing FileWrite...\n\n" RESET);
  EXPECT_TRUE(file_write(1000, kTestString, dstrlen(kTestString)) == FILE_FAIL,
              "Woaah! writing to invalid args");
  EXPECT_TRUE(file_write(-1, kTestString, dstrlen(kTestString)) == FILE_FAIL,
              "Woaah! writing to invalid args");
  EXPECT_TRUE(file_write(10, NULL, dstrlen(kTestString)) == FILE_FAIL,
              "Woaah! writing to invalid args");
  EXPECT_TRUE(file_write(10, kTestString, -1) == FILE_FAIL,
              "Woaah! writing to invalid args");
  EXPECT_TRUE(file_write(14, kTestString, dstrlen(kTestString)) == FILE_FAIL,
              "Woaah! writing to unallocated descriptor");
  EXPECT_TRUE((fd = file_open(kTestFile, "w")) != FILE_FAIL,
              "Failed to open file");
  EXPECT_TRUE(file_write(fd, kTestString, dstrlen(kTestString)) != FILE_FAIL,
              "Failed to write a string");
  EXPECT_TRUE(file_delete(kTestFile) == FILE_SUCCESS, "Failed to delete file");
}

void TestFileRead() {
  const char *kTestFile = "testreadfile.txt";
  char readbuff[4000];
  unsigned int fd;
  Printf(KCYN "\n\n Testing FileRead...\n\n" RESET);
  EXPECT_TRUE(file_read(1000, readbuff, dstrlen(kTestString)) == FILE_FAIL,
              "Woaah! reading from invalid args");
  EXPECT_TRUE(file_read(-1, readbuff, dstrlen(kTestString)) == FILE_FAIL,
              "Woaah! reading from invalid args");
  EXPECT_TRUE(file_read(1000, NULL, dstrlen(kTestString)) == FILE_FAIL,
              "Woaah! reading from invalid args");
  EXPECT_TRUE(file_read(14, readbuff, dstrlen(kTestString)) == FILE_FAIL,
              "Woaah! reading from invalid args");
  EXPECT_TRUE((fd = file_open(kTestFile, "w")) != FILE_FAIL,
              "Failed to open file");
  EXPECT_TRUE(file_write(fd, kTestString, dstrlen(kTestString)) != FILE_FAIL,
              "Failed to write a string");
  EXPECT_TRUE(file_read(fd, readbuff, dstrlen(kTestString)) == FILE_FAIL,
              "Woaah! reading from write only file");
  EXPECT_TRUE(file_close(fd) != FILE_FAIL, "Failed to close file");
  EXPECT_TRUE((fd = file_open(kTestFile, "r")) != FILE_FAIL,
              "Failed to open file");
  EXPECT_TRUE(file_read(fd, readbuff, dstrlen(kTestString)) == FILE_EOF,
              "Woaah! reading from invalid args");
  EXPECT_TRUE(dstrncmp(kTestString, readbuff, dstrlen(kTestString)) == 0,
              "String read was different");
  EXPECT_TRUE(file_delete(kTestFile) == FILE_SUCCESS, "Failed to delete file");
}

void TestFileDelete() {
  Printf(KCYN "\n\n Testing FileDelete...\n\n" RESET);
  EXPECT_TRUE(file_delete("gibberish.txt") == FILE_FAIL,
              "Woah! should not be able to delete non existent file");
  EXPECT_TRUE(file_delete("gibberish2.txt") == FILE_FAIL,
              "Woah ! should not be able to delete non existent file");
}

void TestFileSeek() {
  const char *kTestFile = "testseekfile.txt";
  char readbuff[4000];
  unsigned int fd;
  Printf(KCYN "\n\n Testing FileSeek...\n\n" RESET);
  EXPECT_TRUE(file_seek(1000, 10, FILE_SEEK_END) == FILE_FAIL,
              "Woaah! seeking from invalid handle");
  EXPECT_TRUE(file_seek(-1, 10, FILE_SEEK_END) == FILE_FAIL,
              "Woaah! seeking from invalid handle");
  EXPECT_TRUE(file_seek(-1, 10, -1) == FILE_FAIL,
              "Woaah! seeking from invalid position");
  EXPECT_TRUE(file_seek(14, 10, 1) == FILE_FAIL,
              "Woaah! seeking from unallocated fd");
  EXPECT_TRUE(file_seek(14, 10, 1) == FILE_FAIL,
              "Woaah! seeking from unallocated fd");
  EXPECT_TRUE((fd = file_open(kTestFile, "rw")) != FILE_FAIL,
              "Failed to open file");
  EXPECT_TRUE(file_write(fd, kTestString, dstrlen(kTestString)) != FILE_FAIL,
              "Failed to write a string");
  // Seek set
  EXPECT_TRUE(file_seek(fd, 0, FILE_SEEK_SET) == FILE_SUCCESS,
              "Failed to seek to begining");
  EXPECT_TRUE(file_read(fd, readbuff, dstrlen(kTestString)) == FILE_EOF,
              "Woaah! reading from invalid args");
  EXPECT_TRUE(dstrncmp(kTestString, readbuff, dstrlen(kTestString)) == 0,
              "String read was different");
  // Seek from end
  EXPECT_TRUE(
      file_seek(fd, -dstrlen(kTestString), FILE_SEEK_END) == FILE_SUCCESS,
      "Failed to seek to begining");
  EXPECT_TRUE(file_read(fd, readbuff, dstrlen(kTestString)) == FILE_EOF,
              "Woaah! reading from invalid args");
  EXPECT_TRUE(dstrncmp(kTestString, readbuff, dstrlen(kTestString)) == 0,
              "String read was different");
  // Seek from current
  EXPECT_TRUE(
      file_seek(fd, -dstrlen(kTestString), FILE_SEEK_CUR) == FILE_SUCCESS,
      "Failed to seek to begining");
  EXPECT_TRUE(file_read(fd, readbuff, dstrlen(kTestString)) == FILE_EOF,
              "Woaah! reading from invalid args");
  EXPECT_TRUE(dstrncmp(kTestString, readbuff, dstrlen(kTestString)) == 0,
              "String read was different");
}

void setupTests() {
  REGISTER_TEST(TestFileOpen);
  REGISTER_TEST(TestFileClose);
  REGISTER_TEST(TestFileWrite);
  REGISTER_TEST(TestFileRead);
  REGISTER_TEST(TestFileDelete);
  REGISTER_TEST(TestFileSeek);
}

void runAllTests() {
  int i;
  for (i = 0; i < num_tests; ++i) {
    tests[i]();
  }
}

void main(int argc, char *argv[]) {
  setupTests();
  runAllTests();
  Printf(KGRN
         "\n\n================================================================="
         "=================================\n");
  Printf("File tests:                    %d succeeded, %d failed \n",
         num_tests - num_failures, num_failures);
  Printf(
      "========================================================================"
      "==========================\n\n" RESET);
}
