#include "usertraps.h"
#include "misc.h"
#include "common.h"
#include "utils.h"
void main(int argc, char *argv[]) {
  sem_t sem;
  int num_s2, num_co;
  int num_r1, num_r2, num_r3;
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
  num_r1 = num_s2;
  num_r2 = num_co / 4;
  num_r3 = (int)(num_s2 * 2) > (int)(num_co / 4) ? (int)(num_co / 4)
                                                 : (int)(num_s2 * 2);
  total_procs = num_s2 + num_co + num_r1 + num_r2 + num_r3;
  if ((sem = sem_create(-(total_procs) - 1)) != SYNC_FAIL) {
    LOG("Bad semphore");
    Exit();
  }
  ditoa(s2, s2_str);
  ditoa(co, co_str);
  ditoa(s, s_str);
  ditoa(o2, o2_str);
  ditoa(c2, c2_str);
  ditoa(so4, so4_str);

  if ((s2 = mbox_create()) == MBOX_FAIL) {
    LOG("Error in creating s2 mbox");
    Exit();
  }

  if ((co = mbox_create()) == MBOX_FAIL) {
    LOG("Error in creating co mbox");
    Exit();
  }

  if ((s = mbox_create()) == MBOX_FAIL) {
    LOG("Error in creating s mbox");
    Exit();
  }

  if ((o2 = mbox_create()) == MBOX_FAIL) {
    LOG("Error in creating o2 mbox");
    Exit();
  }

  if ((c2 = mbox_create()) == MBOX_FAIL) {
    LOG("Error in creating c2 mbox");
    Exit();
  }

  if ((so4 = mbox_create()) == MBOX_FAIL) {
    LOG("Error in creating so4 mbox");
    Exit();
  }
  // open
  if (mbox_open(s2) == MBOX_FAIL) {
    LOG("Error in opening s2 mbox");
    Exit();
  }
  if (mbox_open(co) == MBOX_FAIL) {
    LOG("Error in opening co mbox");
    Exit();
  }
  if (mbox_open(s) == MBOX_FAIL) {
    LOG("Error in opening s mbox");
    Exit();
  }
  if (mbox_open(o2) == MBOX_FAIL) {
    LOG("Error in opening o2 mbox");
    Exit();
  }
  if (mbox_open(c2) == MBOX_FAIL) {
    LOG("Error in opening c2 mbox");
    Exit();
  }
  if (mbox_open(so4) == MBOX_FAIL) {
    LOG("Error in opening so4 mbox");
    Exit();
  }

  for (i = 0; i < total_procs; i++) {
    if (i < num_s2) {
      process_create("injector_1.dlx.obj", 0, 0, sem_str, s2_str);
    }
    if (i < num_co) {
      process_create("injector_2.dlx.obj", 0, 0, sem_str, o2_str);
    }
    if (i < num_r1) {
      process_create("reaction_1.dlx.obj", 0, 0, sem_str, s2_str, s_str);
    }
    if (i < num_r2) {
      process_create("reaction_2.dlx.obj", 0, 0, sem_str, co_str, o2_str,
                     c2_str);
    }
    if (i < num_r3) {
      process_create("reaction_3.dlx.obj", 0, 0, sem_str, s_str, o2_str,
                     so4_str);
    }
  }

  if (sem_wait(sem) == SYNC_FAIL) {
    LOG("bad semaphore in %d");
    Exit();
  }
  if (mbox_close(s2) == MBOX_FAIL) {
    LOG("Error in closing s2 mbox");
    Exit();
  }
  if (mbox_close(co) == MBOX_FAIL) {
    LOG("Error in closing co mbox");
    Exit();
  }
  if (mbox_close(s) == MBOX_FAIL) {
    LOG("Error in closing s mbox %d");
    Exit();
  }
  if (mbox_close(o2) == MBOX_FAIL) {
    LOG("Error in closing o2 mbox");
    Exit();
  }
  if (mbox_close(so4) == MBOX_FAIL) {
    LOG("Error in closing so4 mbox");
    Exit();
  }
  if (mbox_close(c2) == MBOX_FAIL) {
    LOG("Error in closing c2 mbox ");
    Exit();
  }

  LOG("All spawned process  done")
}
