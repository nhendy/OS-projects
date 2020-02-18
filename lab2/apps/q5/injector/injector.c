#include "common.h"
#include "utils.h"
#include "usertraps.h"
#include "misc.h"
#include "lab2-api.h"

void printInjectionMessage(const int molecule_idx) {
  switch (molecule_idx) {
    case 0:
      Printf("H2O injected into Radeon atmosphere, PID: %d\n", getpid());
      break;
    case 1:
      Printf("SO4 injected into Radeon atmosphere, PID: %d\n", getpid());
      break;
  }
}

void main(int argc, char** argv) {
  SharedReactionsContext* shared_ctxt;
  uint32 shared_ctxt_handle;
  MoleculeAmountPair molecule_amount_pr;
  sem_t sem;
  int j, molecule_to_inject_idx;
  if (argc < 3) {
    LOG("Too few args in Injector. Exiting...\n");
    Exit();
  }
  shared_ctxt_handle = dstrtol(argv[1], NULL, 10);
  molecule_to_inject_idx = dstrtol(argv[2], NULL, 10);

  if ((shared_ctxt = shmat(shared_ctxt_handle)) == NULL) {
    LOG("Failed to shmat in injector");
    Exit();
  }

  molecule_amount_pr =
      shared_ctxt->injector_ctxt.molecules_to_inject[molecule_to_inject_idx];
  for (j = 0; j < molecule_amount_pr.amount_needed; ++j) {
    sem = lookupSemaphoreByMolecule(shared_ctxt, molecule_amount_pr.molecule);
    semSignalOrDie(sem);
    printInjectionMessage(molecule_to_inject_idx);
  }
  semSignalOrDie(shared_ctxt->all_procs_done_sem);
}
