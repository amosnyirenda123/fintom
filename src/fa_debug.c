#include "../include/fa/fa_debug.h"
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>


void fa_auto_print_metadata(const fa_auto* automaton) {
    if (automaton == NULL) {
        printf("[ERROR] automaton is NULL\n");
        return;
    }

    printf("\n=== Automaton Metadata ===\n");
    printf("States: %d\n", automaton->nstates);
    
    if (automaton->alphabet != NULL) {
        printf("Alphabet size: %d\n", automaton->alphabet->length);
    }
    
    // Count transitions, start states, accept states
    int total_transitions = 0;
    int start_states = 0;
    int accept_states = 0;
    int dead_states = 0;
    
    for (int i = 0; i < automaton->nstates; i++) {
        fa_state* state = automaton->states[i];
        if (state == NULL) continue;
        
        // Count transitions
        int state_transitions = 0;
        fa_trans* current = state->trans;
        while (current != NULL) {
            state_transitions++;
            current = current->next;
        }
        total_transitions += state_transitions;
        
        if (state_transitions == 0) dead_states++;
        if (state->is_start) start_states++;
        if (state->is_accept) accept_states++;
    }
    
    printf("Transitions: %d\n", total_transitions);
    printf("Start states: %d\n", start_states);
    printf("Accept states: %d\n", accept_states);
    printf("Dead states: %d\n", dead_states);
    printf("Density: %.2f transitions/state\n", 
           automaton->nstates > 0 ? (float)total_transitions / automaton->nstates : 0.0);
    printf("==========================\n");
}

void fa_auto_print_metadata_verbose(const fa_auto* automaton) {
    if (automaton == NULL) {
        printf("[ERROR] automaton is NULL\n");
        return;
    }

    printf("\n=== Finite Automaton Metadata ===\n");
    
    // Basic metadata
    printf("Number of states: %d\n", automaton->nstates);
    
    // Alphabet information
    if (automaton->alphabet != NULL) {
        printf("Alphabet size: %d\n", automaton->alphabet->length);
        printf("Alphabet symbols: ");
        set_print(automaton->alphabet, print_string);
        printf("\n");
    } else {
        printf("Alphabet: NULL\n");
    }
    
    // State-by-state analysis
    printf("\n--- State Details ---\n");
    
    int total_transitions = 0;
    int start_states = 0;
    int accept_states = 0;
    
    for (int i = 0; i < automaton->nstates; i++) {
        fa_state* state = automaton->states[i];
        if (state == NULL) {
            printf("  State %d: NULL\n", i);
            continue;
        }
        
        // Count transitions for this state
        int state_transitions = 0;
        fa_trans* current = state->trans;
        while (current != NULL) {
            state_transitions++;
            current = current->next;
        }
        total_transitions += state_transitions;
        
        // Count start/accept states
        if (state->is_start) start_states++;
        if (state->is_accept) accept_states++;
        
        // Print state info
        printf("  State %d: \"%s\"\n", i, state->label ? state->label : "(null)");
        printf("    Start: %s, Accept: %s\n", 
               state->is_start ? "YES" : "NO", 
               state->is_accept ? "YES" : "NO");
        printf("    Outgoing transitions: %d", state_transitions);
        
        // Print ntrans vs actual count for debugging
        if (state_transitions != state->ntrans) {
            printf(" (WARNING: ntrans=%d, actual=%d)", state->ntrans, state_transitions);
        }
        printf("\n");
        
    }
    
    // Summary statistics
    printf("\n--- Summary ---\n");
    printf("Total transitions: %d\n", total_transitions);
    printf("Average transitions per state: %.2f\n", 
           automaton->nstates > 0 ? (float)total_transitions / automaton->nstates : 0.0);
    printf("Start states: %d\n", start_states);
    printf("Accept states: %d\n", accept_states);
    
    // Additional analysis
    printf("\n--- Additional Analysis ---\n");
    
    // Find states with most/least transitions
    int max_transitions = 0;
    int min_transitions = INT_MAX;
    char* max_state_label = NULL;
    char* min_state_label = NULL;
    
    for (int i = 0; i < automaton->nstates; i++) {
        fa_state* state = automaton->states[i];
        if (state == NULL) continue;
        
        int count = 0;
        fa_trans* current = state->trans;
        while (current != NULL) {
            count++;
            current = current->next;
        }
        
        if (count > max_transitions) {
            max_transitions = count;
            max_state_label = state->label;
        }
        if (count < min_transitions) {
            min_transitions = count;
            min_state_label = state->label;
        }
    }
    
    if (max_state_label && min_state_label) {
        printf("State with most transitions: \"%s\" (%d)\n", max_state_label, max_transitions);
        printf("State with fewest transitions: \"%s\" (%d)\n", min_state_label, min_transitions);
    }
    
    // Check for dead states (no outgoing transitions)
    int dead_states = 0;
    printf("Dead states (no outgoing transitions): ");
    for (int i = 0; i < automaton->nstates; i++) {
        fa_state* state = automaton->states[i];
        if (state && state->trans == NULL) {
            printf("\"%s\" ", state->label ? state->label : "(null)");
            dead_states++;
        }
    }
    if (dead_states == 0) printf("None");
    printf("\n");
    
    // Check for unreachable states (would require reverse lookup from start states)
    printf("Automaton type: ");
    if (start_states == 1) {
        printf("Single-start ");
    } else if (start_states > 1) {
        printf("Multiple-start ");
    } else {
        printf("No-start ");
    }
    
    if (accept_states == 0) {
        printf("non-accepting automaton\n");
    } else {
        printf("automaton with %d accepting state(s)\n", accept_states);
    }
    
    printf("================================\n\n");
}

void fa_transitions_print_matrix(const fa_auto* automaton){
    if (automaton == NULL) {
        printf("[ERROR] automaton is NULL\n");
        return;
    }

    printf("\nTRANSITION MATRIX VIEW:\n");
    printf("\n");
    
    // Create a simple matrix representation
    printf("     ");
    for (size_t j = 0; j < automaton->nstates; j++) {
        if (automaton->states[j] != NULL && automaton->states[j]->label != NULL) {
            printf("%-10.10s ", automaton->states[j]->label);
        } else {
            printf("NULL%-6s ", "");
        }
    }
    printf("\n");
    
    for (size_t i = 0; i < automaton->nstates; i++) {
        if (automaton->states[i] != NULL && automaton->states[i]->label != NULL) {
            printf("%-4s ", automaton->states[i]->label);
        } else {
            printf("NULL ");
        }
        
        for (size_t j = 0; j < automaton->nstates; j++) {
            // Check for transitions from state i to state j
            bool has_transition = false;
            if (automaton->states[i] != NULL) {
                fa_trans* trans = automaton->states[i]->trans;
                while (trans != NULL) {
                    if (trans->dest == automaton->states[j]) {
                        has_transition = true;
                        break;
                    }
                    trans = trans->next;
                }
            }
            
            printf("%-10s ", has_transition ? "X" : ".");
        }
        printf("\n");
    }
}


void fa_states_print(const fa_auto* automaton){
    if (automaton == NULL) {
        printf("[ERROR] automaton is NULL\n");
        return;
    }

    printf("\nSTATES:\n");

    for (size_t i = 0; i < automaton->nstates; i++) {
        fa_state* state = automaton->states[i];
        
        if (state == NULL) {
            printf("  State %zu: NULL! (CORRUPTED)\n", i);
            continue;
        }
        
        printf("  State[%zu]:\n", i);
        printf("    Label: '%s' (address: %p)\n", 
               state->label ? state->label : "(null)", 
               (void*)state->label);
        
        printf("    Type: ");
        if (state->is_start && state->is_accept) {
            printf("START & ACCEPT\n");
        } else if (state->is_start) {
            printf("START\n");
        } else if (state->is_accept) {
            printf("ACCEPT\n");
        } else {
            printf("NORMAL\n");
        }
    }
}

void fa_transitions_print(const fa_auto* automaton){
    if (automaton == NULL) {
        printf("[ERROR] automaton is NULL\n");
        return;
    }


    for (size_t i = 0; i < automaton->nstates; i++) {
        fa_state* state = automaton->states[i];

        printf("  State[%zu]:\n", i);
        printf("    Label: '%s'\n", 
               state->label ? state->label : "(null)");
        
        printf("    Out Transitions: %d\n", state->ntrans);
        
        // Print transitions
        fa_trans* trans = state->trans;
        int trans_count = 0;
        while (trans != NULL) {
            printf("      Transition %d:\n", trans_count++);
            printf("        Symbol: '%s' (address: %p)\n", 
                   trans->symbol ? trans->symbol : "(null)",
                   (void*)trans->symbol);
            
            if (trans->dest != NULL) {
                printf("        Destination: '%s' (address: %p)\n",
                       trans->dest->label ? trans->dest->label : "(null)",
                       (void*)trans->dest);
            } else {
                printf("        Destination: NULL (CORRUPTED)\n");
            }
            
            printf("        Next transition: %p\n", (void*)trans->next);
            
            trans = trans->next;
        }
        
        if (trans_count == 0) {
            printf("      No transitions\n");
        }
        
        printf("\n");
    }
    
}

void fa_auto_print_verbose(const fa_auto* automaton) {
    if (automaton == NULL) {
        printf("[ERROR] automaton is NULL\n");
        return;
    }
    
    printf("=== AUTOMATON DEBUG DUMP ===\n\n");
    
    // Basic metadata
    printf("BASIC INFO:\n");
    printf("  Number of states: %zu\n", automaton->nstates);
    
    if (automaton->alphabet != NULL) {
        printf("  Alphabet length: %d\n", automaton->alphabet->length);
        printf("  Alphabet: ");
        
        
        set_print(automaton->alphabet, print_string);
        
        printf("\n");
    } else {
        printf("  Alphabet: NULL\n");
    }
    
    printf("\nSTATES:\n");
    printf("==================================================================\n");
    
    for (size_t i = 0; i < automaton->nstates; i++) {
        fa_state* state = automaton->states[i];
        
        if (state == NULL) {
            printf("  State %zu: NULL! (CORRUPTED)\n", i);
            continue;
        }
        
        printf("  State[%zu]:\n", i);
        printf("    Label: '%s' (address: %p)\n", 
               state->label ? state->label : "(null)", 
               (void*)state->label);
        
        printf("    Type: ");
        if (state->is_start && state->is_accept) {
            printf("START & ACCEPT\n");
        } else if (state->is_start) {
            printf("START\n");
        } else if (state->is_accept) {
            printf("ACCEPT\n");
        } else {
            printf("NORMAL\n");
        }
        
        printf("    Out Transitions: %d\n", state->ntrans);
        
        // Print transitions
        fa_trans* trans = state->trans;
        int trans_count = 0;
        while (trans != NULL) {
            printf("      Transition %d:\n", trans_count++);
            printf("        Symbol: '%s' (address: %p)\n", 
                   trans->symbol ? trans->symbol : "(null)",
                   (void*)trans->symbol);
            
            if (trans->dest != NULL) {
                printf("        Destination: '%s' (address: %p)\n",
                       trans->dest->label ? trans->dest->label : "(null)",
                       (void*)trans->dest);
            } else {
                printf("        Destination: NULL (CORRUPTED)\n");
            }
            
            printf("        Next transition: %p\n", (void*)trans->next);
            
            trans = trans->next;
        }
        
        if (trans_count == 0) {
            printf("      No transitions\n");
        }
        
        printf("\n");
    }
    
    printf("\nMEMORY LAYOUT:\n");
    printf("==================================================================\n");
    printf("  automaton address: %p\n", (void*)automaton);
    printf("  states array: %p\n", (void*)automaton->states);
    
    for (size_t i = 0; i < automaton->nstates; i++) {
        printf("    states[%zu]: %p", i, (void*)automaton->states[i]);
        
        if (automaton->states[i] != NULL) {
            fa_state* state = automaton->states[i];
            printf(" -> label at %p", (void*)state->label);
            
            // Check if label pointer looks valid
            if (state->label != NULL) {
                // Try to see if it's readable (crude check)
                int valid = 1;
                for (int j = 0; j < 10 && state->label[j] != '\0'; j++) {
                    if (state->label[j] < 32 || state->label[j] > 126) {
                        valid = 0;
                        break;
                    }
                }
                printf(" [%s]", valid ? "looks valid" : "SUSPICIOUS");
            }
        }
        printf("\n");
    }
    
    printf("\nTRANSITION MATRIX VIEW:\n");
    printf("==================================================================\n");
    
    // Create a simple matrix representation
    printf("     ");
    for (size_t j = 0; j < automaton->nstates; j++) {
        if (automaton->states[j] != NULL && automaton->states[j]->label != NULL) {
            printf("%-10.10s ", automaton->states[j]->label);
        } else {
            printf("NULL%-6s ", "");
        }
    }
    printf("\n");
    
    for (size_t i = 0; i < automaton->nstates; i++) {
        if (automaton->states[i] != NULL && automaton->states[i]->label != NULL) {
            printf("%-4s ", automaton->states[i]->label);
        } else {
            printf("NULL ");
        }
        
        for (size_t j = 0; j < automaton->nstates; j++) {
            // Check for transitions from state i to state j
            bool has_transition = false;
            if (automaton->states[i] != NULL) {
                fa_trans* trans = automaton->states[i]->trans;
                while (trans != NULL) {
                    if (trans->dest == automaton->states[j]) {
                        has_transition = true;
                        break;
                    }
                    trans = trans->next;
                }
            }
            
            printf("%-10s ", has_transition ? "X" : ".");
        }
        printf("\n");
    }
    
    printf("\nVALIDITY CHECKS:\n");
    printf("==================================================================\n");
    
    int errors = 0;
    
    // Check 1: NULL states in array
    for (size_t i = 0; i < automaton->nstates; i++) {
        if (automaton->states[i] == NULL) {
            printf("  [ERROR] states[%zu] is NULL\n", i);
            errors++;
        }
    }
    
    // Check 2: State label validity
    for (size_t i = 0; i < automaton->nstates; i++) {
        if (automaton->states[i] != NULL) {
            if (automaton->states[i]->label == NULL) {
                printf("  [ERROR] states[%zu]->label is NULL\n", i);
                errors++;
            } else {
                // Check if label appears to be a valid string
                int valid_chars = 0;
                for (int j = 0; j < 100; j++) { // Check first 100 chars
                    if (automaton->states[i]->label[j] == '\0') {
                        valid_chars = 1;
                        break;
                    }
                    if (automaton->states[i]->label[j] < 0) {
                        printf("  [WARNING] states[%zu]->label[%d] has negative value\n", i, j);
                    }
                }
                if (!valid_chars) {
                    printf("  [WARNING] states[%zu]->label has no null terminator in first 100 chars\n", i);
                }
            }
        }
    }
    
    // Check 3: Transition consistency
    for (size_t i = 0; i < automaton->nstates; i++) {
        if (automaton->states[i] != NULL) {
            int counted_trans = 0;
            fa_trans* trans = automaton->states[i]->trans;
            
            while (trans != NULL) {
                counted_trans++;
                
                if (trans->dest == NULL) {
                    printf("  [ERROR] states[%zu]->trans[%d]->dest is NULL\n", i, counted_trans);
                    errors++;
                }
                
                if (trans->symbol == NULL) {
                    printf("  [ERROR] states[%zu]->trans[%d]->symbol is NULL\n", i, counted_trans);
                    errors++;
                }
                
                trans = trans->next;
            }
            
            if (counted_trans != automaton->states[i]->ntrans) {
                printf("  [ERROR] states[%zu]: ntrans=%d but counted %d transitions\n",
                       i, automaton->states[i]->ntrans, counted_trans);
                errors++;
            }
        }
    }
    
    printf("\n=== END DUMP (%d errors found) ===\n\n", errors);
}



void fa_auto_print(const fa_auto* automaton) {
    if (automaton == NULL) {
        printf("Automaton is NULL\n");
        return;
    }
    
    printf("=== QUICK AUTOMATON CHECK ===\n");
    printf("Number of states: %zu\n", automaton->nstates);
    
    printf("\nChecking state labels (CRITICAL FOR YOUR BUG):\n");
    for (size_t i = 0; i < automaton->nstates; i++) {
        fa_state* state = automaton->states[i];
        
        printf("State %zu:\n", i);
        printf("  Address: %p\n", (void*)state);
        
        if (state == NULL) {
            printf("  [CRITICAL] State pointer is NULL!\n");
            continue;
        }
        
        printf("  Label pointer: %p\n", (void*)state->label);
        
        if (state->label == NULL) {
            printf("  [CRITICAL] Label pointer is NULL!\n");
        } else {
            // Try to print the label - this might crash if label points to garbage
            printf("  Label value attempt... ");
            fflush(stdout);
            
            // Use a safe method to check
            int can_print = 1;
            for (int j = 0; j < 50; j++) {
                if (state->label[j] == '\0') {
                    // Found null terminator, string seems valid
                    printf("'%s' (length: %d)\n", state->label, j);
                    can_print = 0;
                    break;
                } else if (state->label[j] < 32 || state->label[j] > 126) {
                    // Non-printable character
                    printf("[INVALID CHAR at pos %d: 0x%02X]\n", j, (unsigned char)state->label[j]);
                    can_print = 0;
                    break;
                }
            }
            
            if (can_print) {
                printf("[NO NULL TERMINATOR in first 50 chars!]\n");
            }
        }
        
        printf("  is_start: %d, is_accept: %d\n", state->is_start, state->is_accept);
        printf("  ntrans: %d\n\n", state->ntrans);
    }
    
    // Check automaton creation
    printf("\n=== DIAGNOSIS ===\n");
    printf("If label pointers look like 0xCCCCCCCC, 0xCDCDCDCD, or other repeating patterns:\n");
    printf("  → Labels were never allocated (your bug!)\n");
    printf("If label pointers are 0x00000000:\n");
    printf("  → Labels were set to NULL\n");
    printf("If label pointers look valid but strings are garbage:\n");
    printf("  → Memory corruption\n");
}