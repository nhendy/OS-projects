#include "usertraps.h"
#include "misc.h"
#include "common.h"
#include "utils.h"

static const char REACTION_STRINGS[][100] = {
    "2H2O = 2H2 + O2", "SO4 = SO2 + O2", "H2 + O2 + SO2 = H2SO4"};

void main(int argc, char** argv) {
  Reaction reactions[sizeof(REACTION_STRINGS) / sizeof(REACTION_STRINGS[0])];
  SharedReactionsContext* shared_ctxt;
  uint32 shared_ctxt_handle;
  int i;
  char shared_ctxt_handle_str[10];

  for (i = 0; i < sizeof(reactions) / sizeof(reactions[0]); ++i) {
    reactions[i] = makeReactionFromString(REACTION_STRINGS[i]);
  }

  if ((shared_ctxt_handle = shmget()) == 0) {
    LOG("Failed to shmget");
    Exit();
  }

  if ((shared_ctxt = shmat(shared_ctxt_handle)) == NULL) {
    LOG("Failed to shmat");
    Exit();
  }

  initCtxt(shared_ctxt);
  fillCtxtFromReactions(shared_ctxt, reactions,
                        sizeof(reactions) / sizeof(reactions[0]));

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

  if ((shared_ctxt->all_procs_done_sem = sem_create(0)) == SYNC_FAIL) {
    LOG("Failed to sem_create all_procs_done_sem");
    Exit();
  }

  process_create(INJECTOR_BINARY, shared_ctxt_handle_str, NULL);
  semWaitOrDie(shared_ctxt->all_procs_done_sem);
}
