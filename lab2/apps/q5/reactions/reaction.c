#include "utils.h"
#include "common.h"
#include "usertraps.h"

void main(int argc, char** argv) {
  SharedReactionsContext* shared_ctxt;
  Reaction* reactions;
  uint32 shared_ctxt_handle, reactions_handle;
  int reaction_idx, i, j;

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

  LOG("Reaction is Up. Executing reaction ");
  printString(reactions[reaction_idx].reaction_string);
  Printf("\n");

  semSignalOrDie(shared_ctxt->all_procs_done_sem);
}
