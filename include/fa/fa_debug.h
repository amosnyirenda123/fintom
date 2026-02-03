#ifndef FA_FA_DEBUG_H
#define FA_FA_DEBUG_H

#include "fa.h"
#include <stdio.h>
#include <stdbool.h>

void fa_transitions_print_matrix(const fa_auto* automaton);
void fa_auto_print_metadata(const fa_auto* automaton);
void fa_auto_print_metadata_verbose(const fa_auto* automaton);
void fa_transitions_print(const fa_auto* automaton);
void fa_states_print(const fa_auto* automaton);
void fa_auto_print_verbose(const fa_auto* automaton);
void fa_auto_print(const fa_auto* automaton);


#endif //FA_FA_DEBUG_H