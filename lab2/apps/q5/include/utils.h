#ifndef __UTILS_HEADER_Q5__
#define __UTILS_HEADER_Q5__
#include "common.h"
// Checks if the character is a digit.
int isDigit(const char c);
// Converts a character to an int.
int charToInt(const char c);
// Prints a string because DLXOS doesn't support string print formatting.
void printString(const char* const str);
// Constructs a string representing a reaction from Reaction object.
DebugString debugString(const Reaction reaction);
// Fills a string with length len with '\0' characters. Useful to make sure
// any string constructed later on is null terminated.
void memsetString(char* const str, const int len);
// Same as above but for all fields of a Reaction object.
void memsetReaction(Reaction* const reaction);
// Initializes a SharedReactionsContext object. Sets string
// fields to zero and all sem_t variables to -1 to serve
// as a sentinel to check whether an entry is being consumed.
void initCtxt(SharedReactionsContext* const shared_ctxt);
// Constructs a Reaction object by parsing a string.
// The reaction string should follow the following simple pseudo regular
// expression.
// SIDE := (NUM_MOLECULES)(MOLECULE) (+ (NUM_MOLECULES)(MOLECULE))*
// REACTION := SIDE = SIDE
Reaction makeReactionFromString(const char* const reaction_str);
// Simple string hash function that works automagically.
int hash(const char* str);
// Lookup molecule based on the above hash in `ctxt` and collision is resolved
// using open addressing by linearly probing the following entries.
sem_t lookupSemaphoreByMolecule(const SharedReactionsContext ctxt,
                                const Molecule molecule);
// Insert the pair in the next available spot in the ctxt based on the hash
// computer by `hash` of the molecule name.
void insertInSharedCtxt(SharedMoleculeSemaphorePair pair,
                        SharedReactionsContext* const ctxt);
#endif
