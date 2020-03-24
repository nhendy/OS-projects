#include "usertraps.h"
#include "misc.h"
#include "common.h"
#include "utils.h"
void main(int argc, char *argv[]) {
  sem_t sem;
  int num_s2, num_co;
  int num_r1, num_r2, num_r3;
  int remaining_s2, remaining_co, remaining_o2, remaining_c2, created_so4,
      remaining_s;
  mbox_t s2, s, co, o2, c2, so4;
  char s2_str[10], s_str[10], co_str[10], o2_str[10], c2_str[10], so4_str[10];
  int total_procs;
  char inj_1_str[10], inj_2_str[10], reaction_1_str[10], reaction_2_str[10],
      reaction_3_str[10];
  int i;
  char sem_str[10];
  int total_args = 3;
  if (argc != total_args) {
    LOG("too few args");
    Exit();
  }

  num_s2 = dstrtol(argv[1], NULL, 10);
  num_co = dstrtol(argv[2], NULL, 10);

  Printf("Creating %d S2s and %d COs.\n", num_s2, num_co);

  num_r1 = num_s2;
  num_r2 = num_co / 4;
  num_r3 = (int)(num_s2 * 2) > (int)(num_co / 4) ? (int)(num_co / 4)
                                                 : (int)(num_s2 * 2);

  total_procs = num_s2 + num_co + num_r1 + num_r2 + num_r3;
  if ((sem = sem_create(-(total_procs - 1))) == SYNC_FAIL) {
    LOG("Bad semphore");
    Exit();
  }

  if ((s2 = mbox_create()) == MBOX_FAIL) {
    Printf("Error in creating S2 in pid %d\n", getpid());
    Exit();
  }

  if ((co = mbox_create()) == MBOX_FAIL) {
    Printf("Error in creating CO in pid %d\n", getpid());
    Exit();
  }

  if ((s = mbox_create()) == MBOX_FAIL) {
    Printf("Error in creating S in pid %d\n", getpid());
    Exit();
  }

  if ((o2 = mbox_create()) == MBOX_FAIL) {
    Printf("Error in creating O2 in pid %d\n", getpid());
    Exit();
  }

  if ((c2 = mbox_create()) == MBOX_FAIL) {
    Printf("Error in creating C2 in pid %d\n", getpid());
    Exit();
  }

  if ((so4 = mbox_create()) == MBOX_FAIL) {
    Printf("Error in creating SO4 in pid %d\n", getpid());
    Exit();
  }

  ditoa(s2, s2_str);
  ditoa(co, co_str);
  ditoa(s, s_str);
  ditoa(o2, o2_str);
  ditoa(c2, c2_str);
  ditoa(so4, so4_str);
  ditoa(sem, sem_str);

  // open
  if (mbox_open(s2) == MBOX_FAIL) {
    Printf("Error in opening S2 in pid %d\n", getpid());
    Exit();
  }
  if (mbox_open(co) == MBOX_FAIL) {
    Printf("Error in opening CO in pid %d\n", getpid());
    Exit();
  }
  if (mbox_open(s) == MBOX_FAIL) {
    Printf("Error in opening S in pid %d\n", getpid());
    Exit();
  }
  if (mbox_open(o2) == MBOX_FAIL) {
    Printf("Error in opening O2 in pid %d\n", getpid());
    Exit();
  }
  if (mbox_open(c2) == MBOX_FAIL) {
    Printf("Error in opening C2 in pid %d\n", getpid());
    Exit();
  }
  if (mbox_open(so4) == MBOX_FAIL) {
    Printf("Error in opening SO4 in pid %d\n", getpid());
    Exit();
  }

  for (i = 0; i < num_s2; i++) {
    process_create("injector_1.dlx.obj", 0, 0, sem_str, s2_str);
  }
  for (i = 0; i < num_co; i++) {
    process_create("injector_2.dlx.obj", 0, 0, sem_str, co_str);
  }
  for (i = 0; i < num_r1; i++) {
    process_create("reaction_1.dlx.obj", 0, 0, sem_str, s2_str, s_str);
  }
  for (i = 0; i < num_r2; i++) {
    process_create("reaction_2.dlx.obj", 0, 0, sem_str, co_str, o2_str, c2_str);
  }
  for (i = 0; i < num_r3; i++) {
    process_create("reaction_3.dlx.obj", 0, 0, sem_str, s_str, o2_str, so4_str);
  }

  if (sem_wait(sem) == SYNC_FAIL) {
    LOG("Bad semaphore in %d");
    Exit();
  }
  if (mbox_close(s2) == MBOX_FAIL) {
    Printf("Error in closing S2 in pid %d\n", getpid());
    Exit();
  }
  if (mbox_close(co) == MBOX_FAIL) {
    Printf("Error in closing CO in pid %d\n", getpid());
    Exit();
  }
  if (mbox_close(s) == MBOX_FAIL) {
    Printf("Error in opening S in pid %d\n", getpid());
    Exit();
  }
  if (mbox_close(o2) == MBOX_FAIL) {
    Printf("Error in opening O2 in pid %d\n", getpid());
    Exit();
  }
  if (mbox_close(so4) == MBOX_FAIL) {
    Printf("Error in closing S04 in pid %d\n", getpid());
    Exit();
  }
  if (mbox_close(c2) == MBOX_FAIL) {
    Printf("Error in closing C2 in pid %d\n", getpid());
    Exit();
  }
  remaining_s = num_s2 * 2 - num_r3;
  remaining_co = num_co - 4 * num_r2;
  remaining_s2 = num_s2 - num_r1;  // always 0
  remaining_o2 = 2 * num_r2 - num_r3 * 2;
  remaining_c2 = num_r2 * 2;
  created_so4 = num_r3;

  Printf(
      "%d CO's left over. %d O2's left over. %d C2's left over. %d S's left "
      "over. %d SO4's created.\n",
      remaining_co, remaining_o2, remaining_c2, remaining_s, created_so4);
}
