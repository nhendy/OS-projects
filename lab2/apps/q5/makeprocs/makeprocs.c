#include "usertraps.h"
#include "misc.h"
#include "common.h"
#include "utils.h"

static const char REACTION_STRINGS[][100] = {
    "2H2O = 2H2 + 1O2", "1SO4 = 1SO2 + 1O2", "1H2 + 1O2 + 1SO2 = 1H2SO4"};

void main(int argc, char** argv) {
  Reaction* reactions;
  SharedReactionsContext* shared_ctxt;
  uint32 shared_ctxt_handle, reactions_handle;
  int i;
  char shared_ctxt_handle_str[10], reactions_handle_str[10],
      reaction_idx_str[10];
  const int kNumReactions =
      sizeof(REACTION_STRINGS) / sizeof(REACTION_STRINGS[0]);

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
    printString(reactions[i].reaction_string);
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

  // TODO: (nhendy) Change this number when all processes are running
  if ((shared_ctxt->all_procs_done_sem = sem_create(0)) == SYNC_FAIL) {
    LOG("Failed to sem_create all_procs_done_sem");
    Exit();
  }

  // Spawn the injector
  process_create(INJECTOR_BINARY, shared_ctxt_handle_str, NULL);

  // Spawn the reaction binaries
  for (i = 0; i < kNumReactions; ++i) {
    ditoa(i, reaction_idx_str);
    process_create(REACTION_BINARY, shared_ctxt_handle_str,
                   reactions_handle_str, reaction_idx_str, NULL);
  }

  semWaitOrDie(shared_ctxt->all_procs_done_sem);
}
