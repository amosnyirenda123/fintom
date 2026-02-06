#include "../include/fa/fa_operations.h"
#include "../include/fa_error.h"
#include "../include/hash/hash_table.h"
#include "../include/common.h"
#include "fa_error.h"
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>




static int partition_get_num_states(const int partition[], const int length) {
    int num_states = 0;

    for (int i = 0; i < length; i++) {
        bool first_occurrence = true;

        for (int j = 0; j < i; j++) {
            if (partition[j] == partition[i]) {
                first_occurrence = false;
                break;
            }
        }

        if (first_occurrence) {
            num_states++;
        }
    }

    return num_states;
}

static fa_auto* generate_minimized_DFA(const fa_auto* automaton, int final_partition[], const int partition_length){

     if (!automaton) {
        return NULL;
    }

    //calculate number states from final partition.
    const int num_states = partition_get_num_states(final_partition, partition_length);
    fa_auto* minimized_dfa = fa_auto_create(num_states);

    //set the alphabet
    minimized_dfa->alphabet = automaton->alphabet; 

    int groups[automaton->capacity];

    for (int i = 0; i < automaton->capacity; i++) {
        groups[i] = -1;
    }

    int state_idx = 0;

    HashTable* rpr_to_group = hash_table_create_string_int(automaton->capacity);
    HashTable* group_to_rpr_minim = hash_table_create_int_string(num_states);
    

    for (int j = 0; j < automaton->capacity; j++) {
        fa_state* current_state = automaton->states[j];
        int current_state_group = final_partition[fa_state_index(automaton, current_state)];
        if (!exists_in_array(groups, automaton->capacity, current_state_group)) {
            char* name;
            snprintf(name, sizeof(name), "q%d", state_idx);
            minimized_dfa->states[state_idx] = fa_state_create(name, false, false);
            groups[j] = current_state_group;

            hash_table_insert(group_to_rpr_minim, &current_state_group, &name);
            state_idx++;
        }
        //Know the group each state belongs to
        hash_table_insert(rpr_to_group, &current_state->label, &current_state_group);
    }
    //Handling Transitions

    for (int i = 0; i < automaton->capacity; i++) {
        fa_state* current_state = automaton->states[i];
        fa_trans* transition = current_state->trans;
        while (transition) {

            fa_state* previous_state = transition->src;
            fa_state* destination_state = transition->dest;

            const char* prev_state_group = hash_table_get(rpr_to_group, &previous_state->label);
            const char* dest_sate_group = hash_table_get(rpr_to_group, &destination_state->label);



            if (prev_state_group && dest_sate_group) {
                const char* min_dfa_state_prev_rpr = hash_table_get(group_to_rpr_minim, &prev_state_group);
                const char* min_dfa_state_dest_rpr = hash_table_get(group_to_rpr_minim, &num_states);


                if (min_dfa_state_prev_rpr && min_dfa_state_dest_rpr) {
                    fa_state* min_dfa_prev_state = fa_state_get_by_label(minimized_dfa, min_dfa_state_prev_rpr);
                    fa_state* min_dfa_dest_state = fa_state_get_by_label(minimized_dfa, min_dfa_state_dest_rpr);

                    if (min_dfa_prev_state && min_dfa_dest_state) {
                        //only add unique transitions
                        if (!fa_auto_has_trans( minimized_dfa, min_dfa_prev_state, transition->symbol, min_dfa_dest_state)) {
                            fa_trans_create(min_dfa_prev_state, min_dfa_dest_state, transition->symbol);
                        }

                    }
                }

            }


            transition = transition->next;
        }
    }

    //deciding final and initial states
    for (int i = 0; i < automaton->capacity; i++) {
        const fa_state* current_state = automaton->states[i];
        if (current_state->is_start) {
            const char* group_idx = hash_table_get(rpr_to_group, &current_state->label);
            if (group_idx) {
                const char* min_dfa_rpr = hash_table_get(group_to_rpr_minim, &group_idx);
                fa_state* min_dfa_state = fa_state_get_by_label(minimized_dfa, min_dfa_rpr);
                min_dfa_state->is_start = 1;
            }
        }

        if (current_state->is_accept) {
            const char* group_idx = hash_table_get(rpr_to_group, &current_state->label);
            if (group_idx) {
                const char* min_dfa_rpr = hash_table_get(group_to_rpr_minim, &group_idx);
                fa_state* min_dfa_state = fa_state_get_by_label(minimized_dfa, min_dfa_rpr);
                min_dfa_state->is_accept = 1;
            }
        }
    }


    hash_table_destroy(rpr_to_group);
    hash_table_destroy(group_to_rpr_minim);

    return minimized_dfa;
}




// Helper Functions
fa_stack* fa_stack_from_args(int count, ...) {
    if (count <= 0) return NULL;
    
    fa_stack* stack = fa_stack_create(count);
    if (!stack) return NULL;
    
    va_list args;
    va_start(args, count);
    
    for (int i = 0; i < count; i++) {
        fa_auto* automaton = va_arg(args, fa_auto*);
        if (!fa_stack_push(stack, automaton)) {
            fa_stack_destroy(stack, false);
            va_end(args);
            return NULL;
        }
    }
    
    va_end(args);
    return stack;
}

fa_stack* fa_stack_from_null_terminated(fa_auto* first, ...) {
    if (!first) return NULL;
    
    fa_stack* stack = fa_stack_create(16);
    if (!stack) return NULL;
    
    // Push first automaton
    if (!fa_stack_push(stack, first)) {
        fa_stack_destroy(stack, false);
        return NULL;
    }
    
    // Process variadic args until NULL
    va_list args;
    va_start(args, first);
    
    fa_auto* automaton;
    while ((automaton = va_arg(args, fa_auto*)) != NULL) {
        if (!fa_stack_push(stack, automaton)) {
            fa_stack_destroy(stack, false);
            va_end(args);
            return NULL;
        }
    }
    
    va_end(args);
    return stack;
}

static fa_auto* compose_single_result(const fa_auto* a, const fa_auto* b,
                                     fa_operation_flags flags) {
    fa_operation_results* results = fa_auto_compose(a, b, flags);
    if (!results || !results->success) {
        if (results) fa_operation_results_destroy(results);
        return NULL;
    }
    
    
    fa_auto* result = NULL;
    
    // Return the first non-NULL result based on flag order
    if (flags & FA_OP_UNION && results->union_result) {
        result = results->union_result;
        
        results->union_result = NULL;
    } else if (flags & FA_OP_INTERSECTION && results->intersection_result) {
        result = results->intersection_result;
        results->intersection_result = NULL;
    } else if (flags & FA_OP_DIFFERENCE && results->difference_result) {
        result = results->difference_result;
        results->difference_result = NULL;
    } else if (flags & FA_OP_CONCATENATION && results->concatenation_result) {
        result = results->concatenation_result;
        results->concatenation_result = NULL;
    }
    
    fa_operation_results_destroy(results);
    return result;
}





// INDIVIDUAL BINARY OPERATIONS

fa_auto* fa_auto_union(const fa_auto* a1, const fa_auto* a2){
    if (!a1 || !a2) return NULL;

    const int offset = 2;
    fa_auto* automaton = fa_auto_create(a1->capacity + a2->capacity + offset);
    automaton->alphabet = set_union(a1->alphabet, a2->alphabet);

    for (int i = 0; i < a1->capacity; i++) {
        automaton->states[i] = fa_state_create(a1->states[i]->label, false, false);
    }

    const int last_idx = a1->capacity;
    for (int i = 0; i < a2->capacity; i++) {
        automaton->states[last_idx + i] = fa_state_create(a2->states[i]->label, false, false);
    }




    //adding a1 transitions
    for (int i = 0; i < a1->capacity; i++) {
        fa_trans* transition = a1->states[i]->trans;
        while (transition) {
            for (int j = 0; j < a1->capacity; j++) {
                if (strcmp(automaton->states[j]->label, transition->dest->label) == 0) {
                    fa_trans_create(automaton->states[i], automaton->states[j], transition->symbol);
                }
            }
            transition = transition->next;
        }
    }



    fa_state *new_origin = fa_state_create("S", true, false);
    fa_state *new_destination = fa_state_create("D", false, true);

    automaton->states[a1->capacity + a2->capacity] = new_origin;
    automaton->states[a1->capacity + a2->capacity + 1] = new_destination;


    //adding a2 transitions
    for (int i = 0; i < a2->capacity; i++) {
        fa_trans* transition = a2->states[i]->trans;
        while (transition) {
            for (int j = 0; j < a2->capacity; j++) {
                if (strcmp(automaton->states[last_idx + j]->label, transition->dest->label) == 0) {
                    fa_trans_create(automaton->states[last_idx + i], automaton->states[last_idx + j], transition->symbol);
                }
            }
            transition = transition->next;
        }
    }

    const int a1_a2_size = a1->capacity + a2->capacity;
    for (int i = 0; i < a1->capacity; i++) {
        if (a1->states[i]->is_start) {
            fa_trans_create(automaton->states[a1_a2_size], automaton->states[i], FA_EPS_SYMBOL);
        }
        if (a1->states[i]->is_accept) {
            fa_trans_create(automaton->states[i], automaton->states[a1_a2_size + 1], FA_EPS_SYMBOL);
        }
    }


    for (int i = 0; i < a2->capacity; i++) {
        if (a2->states[i]->is_start) {
            fa_trans_create(automaton->states[a1_a2_size], automaton->states[last_idx + i], FA_EPS_SYMBOL);
        }
        if (a2->states[i]->is_accept) {
            fa_trans_create(automaton->states[last_idx + i], automaton->states[a1_a2_size + 1], FA_EPS_SYMBOL);
        }
    }

    
    fa_auto_rename_states(automaton);
    return automaton;
}


fa_auto* fa_auto_product(const fa_auto* a1, const fa_auto* a2){
    if (!a1 || !a2) return NULL;

    // Initialisation de l'automate produit
    fa_auto* automaton = fa_auto_create(a1->capacity * a2->capacity);
    if (!automaton) return NULL;

    automaton->alphabet = set_intersection(a1->alphabet, a2->alphabet);

    // Allocation mémoire pour la table des états
    fa_state*** state_automaton = malloc(sizeof(fa_state**) * a1->capacity);
    if (!state_automaton) {
        free(automaton->alphabet);
        free(automaton);
        return NULL;
    }

    for (int i = 0; i < a1->capacity; i++) {
        state_automaton[i] = malloc(sizeof(fa_state*) * a2->capacity);
        if (!state_automaton[i]) {
            for (int k = 0; k < i; k++) free(state_automaton[k]);
            free(state_automaton);
            return NULL;
        }
    }

    // Création des états de l'automate produit
    for (int i = 0; i < a1->capacity; i++) {
        for (int j = 0; j < a2->capacity; j++) {
            char* name_state;
            snprintf(name_state, sizeof(name_state), "%s%s", a1->states[i]->label, a2->states[j]->label);

            int is_starting_state = (a1->states[i]->is_start && a2->states[j]->is_start);
            int is_ending_state = (a1->states[i]->is_accept && a2->states[j]->is_accept);

            state_automaton[i][j] = fa_state_create(name_state, is_starting_state, is_ending_state);
            automaton->states[i * a2->capacity + j] = state_automaton[i][j];
        }
    }

    // Création des transitions
    for (int i = 0; i < a1->capacity; i++) {
        for (int j = 0; j < a2->capacity; j++) {
            fa_state* state1 = a1->states[i];
            fa_state* state2 = a2->states[j];

            fa_trans* transition_node1 = state1->trans;
            while (transition_node1) {
                fa_trans* transition_node2 = state2->trans;
                while (transition_node2) {
                    if (strcmp(transition_node1->symbol, transition_node2->symbol) == 0) {
                        int new_index1_dest = fa_state_index(a1, transition_node1->dest);
                        int new_index2_dest = fa_state_index(a2, transition_node2->dest);

                        int new_index1_prev = fa_state_index(a1, transition_node1->src);
                        int new_index2_prev = fa_state_index(a2, transition_node2->src);

                        if (new_index1_dest == -1 || new_index2_dest == -1 || new_index1_prev == -1 || new_index2_prev == -1) {
                            return NULL;
                        }

                        int new_index_prev = new_index1_prev * a2->capacity + new_index2_prev;
                        int new_index_dest = new_index1_dest * a2->capacity + new_index2_dest;

                        fa_trans* new_transition_node = malloc(sizeof(fa_trans));
                        if (!new_transition_node) {
                            return NULL;
                        }

                        strcpy(new_transition_node->symbol, transition_node1->symbol);
                        new_transition_node->src = automaton->states[new_index_prev];
                        new_transition_node->dest = automaton->states[new_index_dest];

                        // Vérification de cycle
                        fa_trans* temp = automaton->states[i * a2->capacity + j]->trans;
                        while (temp) {
                            if (temp == new_transition_node) {
                                free(new_transition_node);
                                return NULL;
                            }
                            temp = temp->next;
                        }

                        // Ajout de la transition
                        new_transition_node->next = automaton->states[i * a2->capacity + j]->trans;
                        automaton->states[i * a2->capacity + j]->trans = new_transition_node;

                    }
                    transition_node2 = transition_node2->next;
                }
                transition_node1 = transition_node1->next;
            }
        }
    }

    return automaton;
}


fa_auto* fa_auto_concat(const fa_auto* a1, const fa_auto* a2){
    if(!a1 || !a2) return NULL;

    const int offset = 2;
    fa_auto* automaton = fa_auto_create(a1->capacity + a2->capacity);
    automaton->alphabet = set_union(a1->alphabet, a2->alphabet);

    for (int i = 0; i < a1->capacity; i++) {
        automaton->states[i] = fa_state_create(a1->states[i]->label, a1->states[i]->is_start, false);
    }

    const int last_idx = a1->capacity;
    for (int i = 0; i < a2->capacity; i++) {
        automaton->states[last_idx + i] = fa_state_create(a2->states[i]->label, false, a2->states[i]->is_accept);
    }


    //adding a1 transitions
    for (int i = 0; i < a1->capacity; i++) {
        fa_trans* transition = a1->states[i]->trans;
        while (transition) {
            for (int j = 0; j < a1->capacity; j++) {
                if (strcmp(automaton->states[j]->label, transition->dest->label) == 0) {
                    fa_trans_create(automaton->states[i], automaton->states[j], transition->symbol);
                }
            }

            transition = transition->next;
        }
    }


    //adding a2 transitions
    for (int i = 0; i < a2->capacity; i++) {
        fa_trans* transition = a2->states[i]->trans;
        while (transition) {

            for (int j = 0; j < a2->capacity; j++) {
                if (strcmp(automaton->states[last_idx + j]->label, transition->dest->label) == 0) {
                    fa_trans_create(automaton->states[last_idx + i], automaton->states[last_idx + j], transition->symbol);
                }
            }
            transition = transition->next;
        }
    }

    // adding epsilon transitions
     for (int i = 0; i < a1->capacity; i++) {
         if (a1->states[i]->is_accept) {
             for (int j = 0; j < a2->capacity; j++) {
                 if (a2->states[j]->is_start) {
                     fa_trans_create(automaton->states[i], automaton->states[last_idx + j], FA_EPS_SYMBOL);
                 }
             }
         }
    }

    fa_auto_rename_states(automaton);

    return automaton;
}



fa_auto* fa_auto_difference(const fa_auto* a, const fa_auto* b){
    //TODO: Misssing Implementation
    return NULL;
}
fa_auto* fa_auto_symmetric_difference(const fa_auto* a, const fa_auto* b){
    //TODO: Misssing Implementation
    return NULL;
}

// UNARY OPERATIONS
fa_auto* fa_auto_kleene(fa_auto* automaton, fa_kleene_type type){
     if (!automaton) {
        return NULL;
    }

    fa_auto* new_aut = NULL;

    if (type & FA_KLEENE_STAR) {
        const int offset = 2;
        new_aut = fa_auto_create(automaton->capacity + offset);
        new_aut->alphabet = automaton->alphabet;

        fa_state *new_origin = fa_state_create("S", true, false);
        fa_state *new_destination = fa_state_create("D", false, true);

        for (int i = 0; i < automaton->capacity; i++) {
            new_aut->states[i] = automaton->states[i];
        }

        new_aut->states[automaton->capacity] = new_origin;
        new_aut->states[automaton->capacity + 1] = new_destination;

        for (int i = 0; i < automaton->capacity; i++) {
            for (int j = 0; j < automaton->capacity; j++) {
                if (automaton->states[i]->is_start && automaton->states[j]->is_accept) {
                    fa_trans_create(automaton->states[j], automaton->states[i], "eps");
                }
            }
        }


        for (int i = 0; i < automaton->capacity; i++) {
            if (automaton->states[i]->is_start) {
                fa_trans_create(automaton->states[automaton->capacity], automaton->states[i], "eps");
            }

            if (automaton->states[i]->is_accept) {
                fa_trans_create(automaton->states[i], automaton->states[automaton->capacity + 1], "eps");
            }
        }

        fa_trans_create(automaton->states[automaton->capacity], automaton->states[automaton->capacity + 1], "eps");


        for (int i = 0; i < automaton->capacity; i++) {
            automaton->states[i]->is_accept = false;
            automaton->states[i]->is_start = false;
        }



    }
    
    
    if(type & FA_KLEENE_PLUS){
        new_aut = fa_auto_create(automaton->capacity);
        new_aut->alphabet = automaton->alphabet;
        for (int i = 0; i < automaton->capacity; i++) {
            new_aut->states[i] = automaton->states[i];
        }

        for (int i = 0; i < automaton->capacity; i++) {
            for (int j = 0; j < automaton->capacity; j++) {
                if (automaton->states[i]->is_start && automaton->states[j]->is_accept) {
                    fa_trans_create(automaton->states[j], automaton->states[i], "eps");
                }
            }
        }

    }

    return new_aut;
}



fa_auto* fa_auto_minimize_hopcroft(const fa_auto *automaton){
    //TODO: Misssing Implementation
    return NULL;
}
fa_auto* fa_auto_minimize_table(const fa_auto *automaton){
    //TODO: Misssing Implementation
    return NULL;
}
fa_auto* fa_auto_minimize_brzozowski(const fa_auto *automaton){
    //TODO: Misssing Implementation
    return NULL;
}

fa_auto* fa_auto_complement(const fa_auto* a){
    //TODO: Misssing Implementation
    return NULL;
}
fa_auto* fa_auto_reverse(const fa_auto* a){
    //TODO: Misssing Implementation
    return NULL;
}

fa_auto* fa_auto_minimize_moore(const fa_auto *automaton){
    const int N = automaton->capacity;
    int partition[N], new_partition[N];


    for (int i = 0; i < N; i++) {
        partition[i] = automaton->states[i]->is_accept;
    }

    int changed;
    do {
        changed = 0;


        for (int i = 0; i < N; i++) {
            new_partition[i] = partition[i];
        }

        int next_group = 2;
        for (int i = 0; i < N; i++) {
            for (int j = i + 1; j < N; j++) {
                if (partition[i] == partition[j]) {
                    int equivalent = 1;

                    // Check transitions
                    for (const fa_trans *t1 = automaton->states[i]->trans; t1 != NULL; t1 = t1->next) {
                        int found_match = 0;
                        for (const fa_trans *t2 = automaton->states[j]->trans; t2 != NULL; t2 = t2->next) {
                            if (strcmp(t1->symbol, t2->symbol) == 0) {
                                int idx_1 = fa_state_index(automaton, t1->dest);
                                int idx_2 = fa_state_index(automaton, t2->dest);

                                if (partition[idx_1] != partition[idx_2]) {
                                    equivalent = 0;
                                }
                                found_match = 1;
                                break;
                            }
                        }
                        if (!found_match) {
                            equivalent = 0;
                        }
                    }

                    if (!equivalent) {
                        new_partition[j] = N + next_group;
                        changed = 1;
                    }
                }
            }
            next_group++;
        }


        for (int i = 0; i < N; i++) {
            partition[i] = new_partition[i];
        }

    } while (changed);


    fa_auto* minimized_dfa = generate_minimized_DFA(automaton, partition, N);
    return minimized_dfa;
}


fa_auto* fa_auto_minimize(const fa_auto* automaton, fa_minimize_algorithm algorithm) {
    if (!automaton) return NULL;
    
    // // Check if automaton is deterministic
    // if (!fa_auto_is_deterministic(automaton)) {
        
    //     return NULL;
    // }
    
    // Select algorithm based on flags
    if (FA_MINIMIZE_USES_ALGO(algorithm, FA_MINIMIZE_HOPCROFT)) {
        return fa_auto_minimize_hopcroft(automaton);
    }
    else if (FA_MINIMIZE_USES_ALGO(algorithm, FA_MINIMIZE_MOORE)) {
        return fa_auto_minimize_moore(automaton);
    }
    else if (FA_MINIMIZE_USES_ALGO(algorithm, FA_MINIMIZE_TABLE)) {
        return fa_auto_minimize_table(automaton);
    }
    else if (FA_MINIMIZE_USES_ALGO(algorithm, FA_MINIMIZE_BRZOZOWSKI)) {
        return fa_auto_minimize_brzozowski(automaton);
    }
    
    // Default to Hopcroft if no algorithm specified
    return fa_auto_minimize_hopcroft(automaton);
}


fa_auto* fa_auto_determinize(const fa_auto* a, fa_determinize_algorithm algorithm){
    // fa_auto *dfa = fa_auto_create(0);
	// int k =0;
	// SetControl *setcontrol = malloc(sizeof(SetControl));
    // // Initialisation de l'AFD

    // dfa->number_of_states = 0;
    // setcontrol->set_count = 0;
    // memcpy(dfa->alphabet, afn->alphabet, sizeof(afn->alphabet));
    // dfa->alphabet = afn->alphabet;

    // // Création de l'état initial de l'AFD
    // StateSet *initial_set = create_state_set();
    // for (int i = 0; i < afn->number_of_states; i++) {
    //     if (afn->states[i]->is_starting_state && !afn->states[i]->is_ending_state ) {
    //         add_to_state_set(initial_set, afn->states[i]);
    //        // break;
    //     }
    //     if (afn->states[i]->is_starting_state && afn->states[i]->is_ending_state ) {
    //         add_to_state_set(initial_set, afn->states[i]);
    //         k = 1;
    //        // break;
    //     }

    // }
    // generate_set_name(initial_set);
    // setcontrol->state_sets[setcontrol->set_count++] = initial_set;

    // // Créer l'état AFD correspondant
    // State *initial_afd_state = init_state(initial_set->name, true, false);
    // if (k) {
    //     initial_afd_state->is_ending_state = 1;
    // }
    // dfa->states[dfa->number_of_states++] = initial_afd_state;

    // // Pour chaque état de l'AFD
    // for (int i = 0; i < setcontrol->set_count; i++) {
    //     StateSet *current_set = setcontrol->state_sets[i];
    //     State *current_afd_state = dfa->states[i];

    //     // Pour chaque symbole de l'alphabet
    //     for (int j = 0; j < dfa->alphabet->length; j++) {
    //         StateSet *new_set = create_state_set();

    //         // Pour chaque état AFN dans l'ensemble courant
    //         for (int k = 0; k < current_set->count; k++) {
    //             State *afn_state = current_set->states[k];

    //             // Trouver toutes les transitions avec ce symbole
    //             for (TransitionNode *t = afn_state->transition; t != NULL; t = t->next) {
    //                 if (strcmp(t->input_symbol, dfa->alphabet->members[j]) == 0) {
    //                     if (!state_set_contains(new_set, t->destination_state)) {
    //                         add_to_state_set(new_set, t->destination_state);
    //                     }
    //                 }
    //             }
    //         }

    //         if (new_set->count > 0) {
    //             generate_set_name(new_set);

    //             // Vérifier si cet ensemble existe déjà
    //             bool set_exists = false;
    //             State *target_state = NULL;
    //             for (int m = 0; m < setcontrol->set_count; m++) {
    //                 if (strcmp(setcontrol->state_sets[m]->name, new_set->name) == 0) {
    //                     set_exists = true;
    //                     target_state = dfa->states[m];
    //                     break;
    //                 }
    //             }

    //             if (!set_exists) {
    //                 // Créer un nouvel état AFD
    //                 bool is_final = false;
    //                 for (int n = 0; n < new_set->count; n++) {
    //                     if (new_set->states[n]->is_ending_state) {
    //                         is_final = true;
    //                         break;
    //                     }
    //                 }

    //                 State *new_afd_state = init_state(new_set->name, false, is_final);

    //                 setcontrol->state_sets[setcontrol->set_count] = new_set;
    //                 dfa->states[dfa->number_of_states] = new_afd_state;
    //                 target_state = new_afd_state;

    //                 setcontrol->set_count++;
    //                 dfa->number_of_states++;
    //             }

    //             // Ajouter la transition
    //             add_transition(current_afd_state, dfa->alphabet->members[j], target_state);
    //         } else {
    //             free(new_set);
    //         }
    //     }
    // }
    // return rename_states_and_transitions_ret(dfa);
}

fa_auto* fa_auto_optimize(fa_auto* automaton, fa_minimize_algorithm min_algo, fa_determinize_algorithm det_algo){
    if (!automaton) {
        return NULL;
    }


    const fa_auto* det = fa_auto_determinize(automaton, det_algo);
    fa_auto* minim = fa_auto_minimize_hopcroft(det);
    return minim;
}


fa_operation_results* fa_auto_compose(const fa_auto* a, const fa_auto* b, 
                                     fa_operation_flags flags) {
    
    // fa_operation_results* results = calloc(1, sizeof(fa_operation_results));
    // if (!results) {
    //     return NULL;
    // }
    
    // results->success = true;
    // results->performed_ops = 0;
    
    // // Validate inputs for binary operations
    // if ((flags & FA_OP_ALL_BINARY) && (!a || !b)) {
    //     strcpy(results->error_message, "Binary operations require both automata");
    //     results->success = false;
    //     return results;
    // }
    
    // // Validate for unary operations (only need A)
    // if ((flags & FA_OP_ALL_UNARY) && !a) {
    //     strcpy(results->error_message, "Unary operations require at least one automaton");
    //     results->success = false;
    //     return results;
    // }
    
    // // Execute requested operations
    // if (flags & FA_OP_UNION) {
    //     results->union_result = fa_auto_union(a, b);
    //     if (results->union_result) results->performed_ops |= FA_OP_UNION;
    // }
    
    // if (flags & FA_OP_INTERSECTION) {
    //     results->intersection_result = fa_auto_intersection(a, b);
    //     if (results->intersection_result) results->performed_ops |= FA_OP_INTERSECTION;
    // }
    
    
    
    
    // if (flags & FA_OP_CONCATENATION) {
    //     results->concatenation_result = fa_auto_concatenation(a, b);
    //     if (results->concatenation_result) results->performed_ops |= FA_OP_CONCATENATION;
    // }
    
    // // Unary operations (only use A)
    // if (flags & FA_OP_COMPLEMENT) {
    //     results->complement_result = fa_auto_complement(a);
    //     if (results->complement_result) results->performed_ops |= FA_OP_COMPLEMENT;
    // }
    
    // if (flags & FA_OP_REVERSE) {
    //     results->reverse_result = fa_auto_reverse(a);
    //     if (results->reverse_result) results->performed_ops |= FA_OP_REVERSE;
    // }
    

    
    // if (flags & FA_OP_MINIMIZE) {
    //     results->minimized_result = fa_auto_minimize(a, FA_MINIMIZE_FAST);
    //     if (results->minimized_result) results->performed_ops |= FA_OP_MINIMIZE;
    // }
    
    // if (flags & FA_OP_DETERMINIZE) {
    //     results->determinized_result = fa_auto_determinize(a);
    //     if (results->determinized_result) results->performed_ops |= FA_OP_DETERMINIZE;
    // }
    
    // return results;
}



fa_operation_results* fa_auto_stack_compose(fa_stack* stack, 
                                           fa_operation_flags flags, fa_composition_mode mode) {
    if (!stack || fa_stack_is_empty(stack)) {
        return NULL;
    }
    
    // For single automaton, apply unary operations
    if (fa_stack_size(stack) == 1) {
        fa_auto* single = fa_stack_peek(stack);
        return fa_auto_compose(single, NULL, flags & FA_OP_ALL_UNARY);
    }
    
    // Reduce stack by repeatedly composing top two automata
    while (fa_stack_size(stack) > 1) {
        fa_auto* b = fa_stack_pop(stack);
        fa_auto* a = fa_stack_pop(stack);
        
        if (!a || !b) {
            if (a) fa_stack_push(stack, a);
            if (b) fa_auto_destroy(b);
            continue;
        }
        
        // Compose a and b, get single result for binary operation
        fa_auto* result = compose_single_result(a, b, flags);
        
        // Clean up inputs
        fa_auto_destroy(a);
        fa_auto_destroy(b);
        
        // Push result back onto stack
        if (result) {
            fa_stack_push(stack, result);
        } else {
            // Composition failed, cleanup and return NULL
            fa_stack_clear(stack);
            return NULL;
        }
    }
    
    // Final result is on stack
    fa_auto* final_result = fa_stack_pop(stack);
    if (!final_result) return NULL;
    
    // Create operation results containing just the final automaton
    fa_operation_results* results = calloc(1, sizeof(fa_operation_results));
    if (!results) {
        fa_auto_destroy(final_result);
        return NULL;
    }
    
    results->success = true;
    results->performed_ops = flags;
    
    // Store in appropriate field based on operation type
    if (flags & FA_OP_UNION) {
        results->union_result = final_result;
    } else if (flags & FA_OP_INTERSECTION) {
        results->intersection_result = final_result;
    } else if (flags & FA_OP_DIFFERENCE) {
        results->difference_result = final_result;
    } else if (flags & FA_OP_CONCATENATION) {
        results->concatenation_result = final_result;
    } else {
        // Default to union if no specific binary operation
        results->union_result = final_result;
    }
    
    return results;
}


fa_operation_results* fa_auto_ncompose(fa_operation_flags flags,
                                       fa_composition_mode mode, 
                                       int count, ...) {
    if (count <= 0) return NULL;
    
    // Create stack from variadic arguments
    va_list args;
    va_start(args, count);
    
    fa_stack* stack = fa_stack_create(count);
    if (!stack) {
        va_end(args);
        return NULL;
    }
    
    for (int i = 0; i < count; i++) {
        fa_auto* automaton = va_arg(args, fa_auto*);
        if (!fa_stack_push(stack, automaton)) {
            fa_stack_destroy(stack, false);
            va_end(args);
            return NULL;
        }
    }
    
    va_end(args);
    
    // Perform stack composition
    fa_operation_results* results = fa_auto_stack_compose(stack, flags, mode);
    
    // Clean up stack (don't free automata - they're in results)
    fa_stack_destroy(stack, false);
    
    return results;
}

fa_operation_results* fa_auto_vcompose(fa_operation_flags flags,
                                       fa_composition_mode mode, 
                                       fa_auto* first, ...) {
    if (!first) return NULL;
    
    va_list args;
    va_start(args, first);
    
    fa_stack* stack = fa_stack_create(8);
    if (!stack) {
        va_end(args);
        return NULL;
    }
    
    // Push first automaton
    if (!fa_stack_push(stack, first)) {
        fa_stack_destroy(stack, false);
        va_end(args);
        return NULL;
    }
    
    // Process remaining arguments until NULL
    fa_auto* automaton;
    while ((automaton = va_arg(args, fa_auto*)) != NULL) {
        if (!fa_stack_push(stack, automaton)) {
            fa_stack_destroy(stack, false);
            va_end(args);
            return NULL;
        }
    }
    
    va_end(args);
    
    // Perform stack composition
    fa_operation_results* results = fa_auto_stack_compose(stack, flags, mode);
    
    // Clean up stack
    fa_stack_destroy(stack, false);
    
    return results;
}


void fa_operation_results_destroy(fa_operation_results* results) {
    if (!results) return;
    
    if (results->union_result) fa_auto_destroy(results->union_result);
    if (results->intersection_result) fa_auto_destroy(results->intersection_result);
    if (results->difference_result) fa_auto_destroy(results->difference_result);
    if (results->symmetric_difference_result) fa_auto_destroy(results->symmetric_difference_result);
    if (results->concatenation_result) fa_auto_destroy(results->concatenation_result);
    if (results->complement_result) fa_auto_destroy(results->complement_result);
    if (results->reverse_result) fa_auto_destroy(results->reverse_result);
    if (results->kleene_star_result) fa_auto_destroy(results->kleene_star_result);
    if (results->minimized_result) fa_auto_destroy(results->minimized_result);
    if (results->determinized_result) fa_auto_destroy(results->determinized_result);
    
    free(results);
}

