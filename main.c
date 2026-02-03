#include <stdlib.h>
#include <stdio.h>
#include "include/fa/fa.h"          // Core finite automaton structures & functions
#include "include/set/set.h"        // Generic set implementation (used for alphabet)
#include "include/io/fa_auto_io.h"  // Printing / I/O helpers for automata
#include "include/fa_error.h"       // Error codes returned by FA functions
#include "include/common.h"
#include "include/fa/fa_debug.h"

int main()
{
    /* -----------------------------
       1. Define basic automaton size
       ----------------------------- */

    const int num_states = 4;   // We will create an automaton with 4 states

    /* -----------------------------
       2. Define the alphabet symbols
       ----------------------------- */

    // List of input symbols the automaton understands
    const char* symbols[] = {"a", "b", "c", "d", "e"};

    // Compute number of symbols automatically
    const int nsymbols = sizeof(symbols) / sizeof(symbols[0]);

    // Create a set that will store strings (our alphabet container)
    Set* alphabet = set_create_string_set();

    // Insert all symbols into the alphabet set
    // Returns number of successfully inserted symbols
    size_t count = fa_alphabet_insert_symbols(alphabet, symbols, nsymbols);

    

    /* -----------------------------
       3. Create states
       fa_state_create(label, is_start, is_accepting)
       ----------------------------- */

    fa_state* q0 = fa_state_create("q0", 1, 1); // Start state AND accepting
    fa_state* q1 = fa_state_create("q1", 1, 0); // Start state (if NFA), not accepting
    fa_state* q2 = fa_state_create("q2", 0, 1); // Accepting state
    fa_state* q3 = fa_state_create("q3", 0, 1); // Accepting state

    /* -----------------------------
       4. Create automaton container
       ----------------------------- */

    // Allocate automaton structure with space for 4 states
    fa_auto* automaton = fa_auto_create(num_states);

    // Attach alphabet to automaton
    automaton->alphabet = alphabet;

    // Register states inside the automaton
    automaton->states[0] = q0;
    automaton->states[1] = q1;
    automaton->states[2] = q2;
    automaton->states[3] = q3;

    

    /* -----------------------------
       5. Define transitions (data only)
       These are NOT yet inside the automaton
       ----------------------------- */

    fa_trans transitions[] = {
        {.src = q0, .dest = q0, .symbol = "a", .next = NULL}, // q0 --a--> q0 (self-loop)
        {.src = q0, .dest = q1, .symbol = "e", .next = NULL}, // q0 --e--> q1
        {.src = q0, .dest = q2, .symbol = "b", .next = NULL}, // q0 --b--> q2
        {.src = q0, .dest = q3, .symbol = "c", .next = NULL}, // q0 --c--> q3
        {.src = q1, .dest = q2, .symbol = "d", .next = NULL}, // q1 --d--> q2
        {.src = q2, .dest = q3, .symbol = "c", .next = NULL}, // q2 --c--> q3
        {.src = q3, .dest = q2, .symbol = "e", .next = NULL}, // q3 --e--> q2
        {.src = q3, .dest = q3, .symbol = "a", .next = NULL}  // q3 --a--> q3 (self-loop)
    };

    // Number of transitions in the array
    size_t num_transitions = sizeof(transitions) / sizeof(transitions[0]);

    /* -----------------------------
       6. Insert transitions into FA
       with validation
       ----------------------------- */
    
    for (size_t i = 0; i < num_transitions; i++) {

        // Try to create and attach transition to the automaton
        // Validation checks typically include:
        // - symbol exists in alphabet
        // - states belong to this automaton
        fa_error_t err = fa_trans_create_validated(
                            automaton,
                            transitions[i].src,
                            transitions[i].dest,
                            transitions[i].symbol);

        
        
        // If something went wrong, print a readable error
        if (err != FA_SUCCESS) {

            fprintf(stderr, "Transition %s->%s ('%s') failed: %s\n",
                    transitions[i].src->label,
                    transitions[i].dest->label,
                    transitions[i].symbol,
                    fa_error_str(err));

            
        }
    }


    /* -----------------------------
       7. Generate the corresponding dot file
       ----------------------------- */
    fa_auto_export_dot_file(automaton, "../examples/basic_automaton.dot");


    /* -----------------------------
       8. Generate the corresponding json file
       ----------------------------- */
    fa_auto_export_json_file(automaton, "../examples/basic_automaton.json");
    

    /* -----------------------------
       9. Print the resulting automaton
       ----------------------------- */

    // Print full automaton structure using a function that knows
    // how to print symbols stored as strings
    fa_auto_print_verbose(automaton);



    // Destroy automaton
    fa_auto_destroy(automaton);
    
}
