#include "utils.h"
#include "common.h"
#include "usertraps.h"
#include "misc.h"

void main(int argc, char** argv) {
  SharedReactionsContext* shared_ctxt;
  Reaction* reactions;
  uint32 shared_ctxt_handle, reactions_handle;
  int reaction_idx, i, j, k;
  sem_t sem;
  Molecule molecule;

  if (argc < 4) {
    LOG("Too few args in Reactions. Exiting...\n");
    Printf("Expected %d, got  %d\n", 4, argc);
    Exit();
  }

  shared_ctxt_handle = dstrtol(argv[1], NULL, 10);
  reactions_handle = dstrtol(argv[2], NULL, 10);
  reaction_idx = dstrtol(argv[3], NULL, 10);

  if ((shared_ctxt = shmat(shared_ctxt_handle)) == NULL) {
    LOG("Failed to shmat shared_ctxt_handle in reaction");
    Exit();
  }

  if ((reactions = shmat(reactions_handle)) == NULL) {
    LOG("Failed to shmat reactions_handle in reaction");
    Exit();
  }

  for (k = 0; k < reactions[reaction_idx].num_occurrences; ++k) {
    for (i = 0; i < reactions[reaction_idx].num_inputs; ++i) {
      for (j = 0; j < reactions[reaction_idx].inputs[i].amount_needed; ++j) {
        molecule = reactions[reaction_idx].inputs[i].molecule;
        sem = lookupSemaphoreByMolecule(shared_ctxt, molecule);
        semWaitOrDie(sem);
      }
    }
    for (i = 0; i < reactions[reaction_idx].num_outputs; ++i) {
      for (j = 0; j < reactions[reaction_idx].outputs[i].amount_needed; ++j) {
        molecule = reactions[reaction_idx].outputs[i].molecule;
        sem = lookupSemaphoreByMolecule(shared_ctxt, molecule);
        semSignalOrDie(sem);
      }
    }
    GUARD_EXPRESSIONS(
        shared_ctxt->print_lock,
        printString(reactions[reaction_idx].reaction_string),
        Printf("Reaction %d reacted. PID: %d\n", reaction_idx, getpid()));
  }
  semSignalOrDie(shared_ctxt->all_procs_done_sem);
}
