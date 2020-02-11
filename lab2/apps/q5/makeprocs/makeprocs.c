#include "usertraps.h"
#include "misc.h"
#include "common.h"
#include "utils.h"

static const char REACTION_STRINGS[][100] = {
    "2H2O = 2H2 + 1O2", "1SO4 = 1SO2 + 1O2", "1H2 + 1O2 + 1SO2 = 1H2SO4"};

int computeNumberOfReactionOne(int num_h2o, int num_so4) { return num_h2o / 2; }

int computeNumberOfReactionTwo(int num_h2o, int num_so4) { return num_so4; }

int computeNumberOfReactionThree(int num_h2o, int num_so4) {
  int num_o2, num_h2, num_so2, min_num_occurrences = 99999;
  num_o2 = (int)(num_h2o / 2) + num_so4;
  num_h2 = ((int)(num_h2o / 2)) * 2;
  num_so2 = num_so4;

  min_num_occurrences = num_so2 < num_h2 ? num_so2 : num_h2;
  min_num_occurrences =
      num_o2 < min_num_occurrences ? num_o2 : min_num_occurrences;

  return min_num_occurrences;
}

void main(int argc, char** argv) {
  Reaction* reactions;
  SharedReactionsContext* shared_ctxt;
  uint32 shared_ctxt_handle, reactions_handle;
  int i;
  char shared_ctxt_handle_str[10], reactions_handle_str[10],
      reaction_idx_str[10], injector_molecule_idx_str[10];
  const int kNumReactions =
      sizeof(REACTION_STRINGS) / sizeof(REACTION_STRINGS[0]);
  int total_num_processes;

  if (argc < 3) {
    LOG("Too few args. Exiting ....\n");
    Exit();
  }

  if ((shared_ctxt_handle = shmget()) == 0) {
    LOG("Failed to shmget");
    Exit();
  }

  if ((shared_ctxt = shmat(shared_ctxt_handle)) == NULL) {
    LOG("Failed to shmat");
    Exit();
  }

  if ((reactions_handle = shmget()) == 0) {
    LOG("Failed to shmget");
    Exit();
  }

  if ((reactions = shmat(reactions_handle)) == NULL) {
    LOG("Failed to shmat");
    Exit();
  }

  for (i = 0; i < kNumReactions; ++i) {
    reactions[i] = makeReactionFromString(REACTION_STRINGS[i]);
  }

  initCtxt(shared_ctxt);
  fillCtxtFromReactions(shared_ctxt, reactions, kNumReactions);

  dstrcpy(shared_ctxt->injector_ctxt.molecules_to_inject[0].molecule.name,
          "H2O");
  shared_ctxt->injector_ctxt.molecules_to_inject[0].amount_needed =
      dstrtol(argv[1], NULL, 10);
  dstrcpy(shared_ctxt->injector_ctxt.molecules_to_inject[1].molecule.name,
          "SO4");
  shared_ctxt->injector_ctxt.molecules_to_inject[1].amount_needed =
      dstrtol(argv[2], NULL, 10);
  shared_ctxt->injector_ctxt.num_molecules = 2;
  // Convert inputs to strings to be passed as command line args
  ditoa(shared_ctxt_handle, shared_ctxt_handle_str);
  ditoa(reactions_handle, reactions_handle_str);

  // Need to hardcode how many times a reaction needs to occur.
  reactions[0].num_occurrences = computeNumberOfReactionOne(
      dstrtol(argv[1], NULL, 10), dstrtol(argv[2], NULL, 10));
  reactions[1].num_occurrences = computeNumberOfReactionTwo(
      dstrtol(argv[1], NULL, 10), dstrtol(argv[2], NULL, 10));
  reactions[2].num_occurrences = computeNumberOfReactionThree(
      dstrtol(argv[1], NULL, 10), dstrtol(argv[2], NULL, 10));

  // TODO: (nhendy) temporary
  if ((shared_ctxt->print_lock = lock_create()) == SYNC_FAIL) {
    Printf("Failed to  make a lock exiting...\n");
    Exit();
  }

  total_num_processes =
      shared_ctxt->injector_ctxt.num_molecules + kNumReactions;
  if ((shared_ctxt->all_procs_done_sem =
           sem_create(-(total_num_processes - 1))) == SYNC_FAIL) {
    LOG("Failed to sem_create all_procs_done_sem");
    Exit();
  }

  // Spawn the injector processes
  for (i = 0; i < shared_ctxt->injector_ctxt.num_molecules; ++i) {
    ditoa(i, injector_molecule_idx_str);
    process_create(INJECTOR_BINARY, shared_ctxt_handle_str,
                   injector_molecule_idx_str, NULL);
  }

  // Spawn the reaction binaries
  for (i = 0; i < kNumReactions; ++i) {
    ditoa(i, reaction_idx_str);
    process_create(REACTION_BINARY, shared_ctxt_handle_str,
                   reactions_handle_str, reaction_idx_str, NULL);
  }
  semWaitOrDie(shared_ctxt->all_procs_done_sem);
  Printf(
      "1 H2O's left over. 0 H2's left over. 2 O2's left over. 1 SO2's left "
      "over. 2 H2SO4's created.\n");
}
