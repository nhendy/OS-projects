#include "common.h"
#include "utils.h"
#include "usertraps.h"
#include "misc.h"
#include "lab2-api.h"

void main(int argc, char** argv) {
  SharedReactionsContext* shared_ctxt;
  uint32 shared_ctxt_handle;
  MoleculeAmountPair molecule_amount_pr;
  sem_t sem;
  int i, j;
  if (argc < 2) {
    LOG("Too few args in Injector. Exiting...\n");
    Exit();
  }
  LOG("Injector is Up\n");
  shared_ctxt_handle = dstrtol(argv[1], NULL, 10);
  if ((shared_ctxt = shmat(shared_ctxt_handle)) == NULL) {
    LOG("Failed to shmat in injector");
    Exit();
  }

  for (i = 0; i < shared_ctxt->injector_ctxt.num_molecules; i++) {
    molecule_amount_pr = shared_ctxt->injector_ctxt.molecules_to_inject[i];
    for (j = 0; j < molecule_amount_pr.amount_needed; ++j) {
      sem = lookupSemaphoreByMolecule(shared_ctxt, molecule_amount_pr.molecule);
      semSignalOrDie(sem);
      //   Printf("Injected ");
      //   printString(molecule_amount_pr.molecule.name);
      //   Printf("\n");
    }
  }
  semSignalOrDie(shared_ctxt->all_procs_done_sem);
}
