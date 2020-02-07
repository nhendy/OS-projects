#ifndef __COMMON_HEADER_Q5__
#define __COMMON_HEADER_Q5__
#include "lab2-api.h"

// Macros for test truthy/falsey return values
#define FALSE 0
#define TRUE 1
// Maros for invalid semaphore values and amount_needed
// variables. Useful as sentinels that an object
// wasn't fully constructed yet.
#define INVALID_SEMAPHORE_CONST -1
#define INVALID_AMOUNT_NEEDED_CONST -1
// Maximum number of molecules allowed.
#define NUM_MOLECULES 100
// Names of children processes.
#define INJECTOR_BINARY "injector.dlx.obj"
#define REACTION_BINARY "reaction.dlx.obj"
// Struct representing a molecule. Currently
// only holds its name.
typedef struct {
  char name[50];
} Molecule;
// Wrapper struct just to allow functions
// to return char arrays.
typedef struct {
  char str[100];
} DebugString;
// Wrapper struct used to group molecule with its
// associated amount needed. Useful for Reaction object.
typedef struct {
  Molecule molecule;
  int amount_needed;
} MoleculeAmountPair;
// Struct representing a Reaction. It contains
// information about molecules neeeded for input
// and output and their corresponding amounts.
typedef struct {
  MoleculeAmountPair inputs[10];
  MoleculeAmountPair outputs[10];
  int num_inputs;
  int num_outputs;
  char reaction_string[100];
} Reaction;
// Context object used to inform the injector of what and how many
// molecules to inject.
typedef struct {
  MoleculeAmountPair molecules_to_inject[5];
  int num_molecules;
} InjectorContext;
// Wrapper struct to group molecule with its associated
// semaphore.
typedef struct {
  Molecule molecule;
  sem_t sem;
} SharedMoleculeSemaphorePair;
// Struct containing a list of molecules and their corresponding
// semaphore and a global semaphore used to signal that all children
// processes are done.
typedef struct {
  SharedMoleculeSemaphorePair molecule_sems[NUM_MOLECULES];
  InjectorContext injector_ctxt;
  int len_molecule_sems;
  sem_t all_procs_done_sem;
} SharedReactionsContext;
#endif
