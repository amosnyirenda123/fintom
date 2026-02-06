#ifndef FA_FA_DEBUG_H
#define FA_FA_DEBUG_H

#include "fa.h"
#include "fa_operations.h"
#include <stdio.h>
#include <stdbool.h>

void fa_transitions_print_matrix(const fa_auto* automaton);
void fa_auto_print_metadata(const fa_auto* automaton);
void fa_auto_print_metadata_verbose(const fa_auto* automaton);
void fa_transitions_print(const fa_auto* automaton);
void fa_states_print(const fa_auto* automaton);
void fa_auto_print_verbose(const fa_auto* automaton);
void fa_auto_print(const fa_auto* automaton);
void fa_operation_results_print(const fa_operation_results* results);


#endif //FA_FA_DEBUG_H