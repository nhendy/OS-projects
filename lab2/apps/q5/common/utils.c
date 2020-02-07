#include "common.h"
#include "utils.h"
#include "misc.h"
#include "usertraps.h"

int isDigit(const char c) {
  if ((c >= '0') && (c <= '9')) return TRUE;
  return FALSE;
}

int charToInt(const char c) { return c - '0'; }

DebugString debugString(const Reaction reaction) {
  DebugString result;
  int i;
  char buffer[10];
  dstrcpy(result.str, "Inputs are: ");
  for (i = 0; i < reaction.num_inputs; ++i) {
    ditoa(reaction.inputs[i].amount_needed, buffer); /*  */
    dstrcat(result.str, buffer);
    dstrcat(result.str, reaction.inputs[i].molecule.name);
    if (i != reaction.num_inputs - 1) dstrcat(result.str, ", ");
  }
  dstrcat(result.str, ". Outputs are: ");
  for (i = 0; i < reaction.num_outputs; ++i) {
    ditoa(reaction.outputs[i].amount_needed, buffer);
    dstrcat(result.str, buffer);
    dstrcat(result.str, reaction.outputs[i].molecule.name);
    if (i != reaction.num_outputs - 1) dstrcat(result.str, ", ");
  }
  return result;
}

void printString(const char* const str) {
  while (*str != '\0') {
    Printf("%c", *str);
    str++;
  }
}

void memsetString(char* const str, const int len) {
  int i;
  for (i = 0; i < len; ++i) {
    str[i] = '\0';
  }
}

// Helper function to ensure that all strings are zeroed out.
void memsetReaction(Reaction* const reaction) {
  int i;
  int size, name_len;
  size = sizeof(reaction->inputs) / sizeof(reaction->inputs[0]);
  for (i = 0; i < size; ++i) {
    name_len = sizeof(reaction->inputs[i].molecule.name);
    reaction->inputs[i].amount_needed = INVALID_AMOUNT_NEEDED_CONST;
    memsetString(reaction->inputs[i].molecule.name, name_len);
  }
  size = sizeof(reaction->outputs) / sizeof(reaction->outputs[0]);
  for (i = 0; i < size; ++i) {
    name_len = sizeof(reaction->outputs[i].molecule.name);
    reaction->outputs[i].amount_needed = INVALID_AMOUNT_NEEDED_CONST;
    memsetString(reaction->outputs[i].molecule.name, name_len);
  }
  name_len = sizeof(reaction->reaction_string);
  memsetString(reaction->reaction_string, name_len);
}

void initCtxt(SharedReactionsContext* const shared_ctxt) {
  int i, size, len;
  shared_ctxt->all_procs_done_sem = INVALID_SEMAPHORE_CONST;
  size = sizeof(shared_ctxt->molecule_sems) /
         sizeof(shared_ctxt->molecule_sems[0]);
  for (i = 0; i < size; ++i) {
    len = sizeof(shared_ctxt->molecule_sems[i].molecule.name);
    shared_ctxt->molecule_sems[i].sem = INVALID_SEMAPHORE_CONST;
    memsetString(shared_ctxt->molecule_sems[i].molecule.name, len);
  }
  shared_ctxt->len_molecule_sems = 0;
}

void insertInSharedCtxt(SharedMoleculeSemaphorePair pair,
                        SharedReactionsContext* const ctxt) {
  int key, size;
  size = sizeof(ctxt->molecule_sems) / sizeof(ctxt->molecule_sems[0]);
  key = hash(pair.molecule.name) % size;
  while (ctxt->molecule_sems[key].sem != INVALID_SEMAPHORE_CONST) {
    key++;
  }
  ctxt->molecule_sems[key] = pair;
  ctxt->len_molecule_sems++;
}

void fillCtxtFromReactions(SharedReactionsContext* const shared_ctxt,
                           const Reaction* const reactions, const int len) {
  int i, j;
  SharedMoleculeSemaphorePair pair;

  for (i = 0; i < len; ++i) {
    // TODO: (nhendy) inputs and outputs are programmatically the same
    // maybe diff using a index only. Too error prone!!!
    for (j = 0; j < reactions[i].num_inputs; ++j) {
      dstrcpy(pair.molecule.name, reactions[i].inputs[j].molecule.name);
      if (lookupSemaphoreByMolecule(shared_ctxt, pair.molecule) !=
          INVALID_SEMAPHORE_CONST)
        continue;
      if ((pair.sem = sem_create(0)) == SYNC_FAIL) {
        LOG("Failed to sem_create. Exiting...");
        Exit();
      }
      insertInSharedCtxt(pair, shared_ctxt);
    }
    for (j = 0; j < reactions[i].num_outputs; ++j) {
      dstrcpy(pair.molecule.name, reactions[i].outputs[j].molecule.name);
      if (lookupSemaphoreByMolecule(shared_ctxt, pair.molecule) !=
          INVALID_SEMAPHORE_CONST)
        continue;
      if ((pair.sem = sem_create(0)) == SYNC_FAIL) {
        LOG("Failed to sem_create. Exiting...");
        Exit();
      }
      insertInSharedCtxt(pair, shared_ctxt);
    }
  }
  Printf("Total length of molecule-semaphore pairs %d\n",
         shared_ctxt->len_molecule_sems);
}

Reaction makeReactionFromString(const char* const reaction_str) {
  Reaction reaction;
  char* curr_pos = reaction_str;
  // Indices used to track which atom is being parsed
  // and which molecule.
  int molecule_idx = 0, atom_idx = 0;
  char curr_char;
  // Simple two state machine for parsing
  enum {
    PARSING_INPUT_MOLECULE,
    PARSING_OUTPUT_MOLECULE
  } state;
  memsetReaction(&reaction);
  state = PARSING_INPUT_MOLECULE;
  dstrcpy(reaction.reaction_string, reaction_str);
  // TODO: (nhendy) simplify this state machine a bit.
  while (*curr_pos != '\0') {
    curr_char = *curr_pos;
    if (state == PARSING_INPUT_MOLECULE && !isspace(curr_char)) {
      if (curr_char == '+') {
        molecule_idx++;
        atom_idx = 0;
      } else if (curr_char == '=') {
        reaction.num_inputs = molecule_idx + 1;
        molecule_idx = 0;
        atom_idx = 0;
        state = PARSING_OUTPUT_MOLECULE;
      } else if (isDigit(curr_char) == TRUE && atom_idx == 0) {
        reaction.inputs[molecule_idx].amount_needed = charToInt(curr_char);
      } else {
        reaction.inputs[molecule_idx].molecule.name[atom_idx++] = curr_char;
      }
    } else if (state == PARSING_OUTPUT_MOLECULE && !isspace(curr_char)) {
      if (curr_char == '+') {
        molecule_idx++;
        atom_idx = 0;
      } else if (isDigit(curr_char) == TRUE && atom_idx == 0) {
        reaction.outputs[molecule_idx].amount_needed = charToInt(curr_char);
      } else {
        reaction.outputs[molecule_idx].molecule.name[atom_idx++] = curr_char;
      }
    }
    ++curr_pos;
  }
  // Number of output molecules is the index we ended at plus 1.
  reaction.num_outputs = molecule_idx + 1;
  return reaction;
}

int hash(const char* str) {
  int hash = 5381;
  int c;
  while (c = *str++) hash = ((hash << 5) + hash) + c;
  return hash;
}

sem_t lookupSemaphoreByMolecule(const SharedReactionsContext* const ctxt,
                                const Molecule molecule) {
  int key, size;
  size = sizeof(ctxt->molecule_sems) / sizeof(ctxt->molecule_sems[0]);
  key = hash(molecule.name) % size;
  while (dstrncmp(ctxt->molecule_sems[key].molecule.name, molecule.name,
                  sizeof(molecule.name)) != 0 &&
         key < size) {
    key = (key + 1);
  }
  if (key >= size) {
    return INVALID_SEMAPHORE_CONST;
  }
  return ctxt->molecule_sems[key].sem;
}

void semSignalOrDie(sem_t sem) {
  if (sem_signal(sem) == SYNC_FAIL) {
    Printf("Weird. Couldn't sem_signal sem: %d, pid: %d\n", sem, getpid());
    Exit();
  }
}

void semWaitOrDie(sem_t sem) {
  if (sem_wait(sem) == SYNC_FAIL) {
    Printf("Weird. Couldn't sem_wait sem: %d, pid: %d\n", sem, getpid());
    Exit();
  }
}
