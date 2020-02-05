#include "usertraps.h"
#include "misc.h"
#include "common.h"
#include "utils.h"

static const char REACTION_STRINGS[][100] = {
    "2H2O = 2H2 + O2", "SO4 = SO2 + O2", "H2 + O2 + SO2 = H2SO4"};

int main(int argc, char** argv) {
  Reaction reactions[sizeof(REACTION_STRINGS) / sizeof(REACTION_STRINGS[0])];
  SharedReactionsContext shared_ctxt;
  uint32 shared_ctxt_handle;
  int i;

  for (i = 0; i < sizeof(reactions) / sizeof(reactions[0]); ++i) {
    reactions[i] = makeReactionFromString(REACTION_STRINGS[i]);
  }

  initCtxt(&shared_ctxt);
  fillCtxtFromReactions(&shared_ctxt, reactions,
                        sizeof(reactions) / sizeof(reactions[0]));
  return 0;
}
