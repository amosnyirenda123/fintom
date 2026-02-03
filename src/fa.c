#include "../include/fa/fa.h"
#include "../include/set/set.h"
#include "../include/common.h"
#include "../include/hash/hash_table.h"
#include "../include/regex/regexpr.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define FA_STACK_DEFAULT_CAPACITY 16
#define FA_EPS_SYMBOL "eps"


size_t fa_alphabet_insert_symbols(Set* alphabet, const char* symbols[], size_t count){
    if(!alphabet || !symbols || count == 0) return 0;

    size_t inserted_count = 0;

    for (size_t i = 0; i < count; i++) {
        if(symbols[i] == NULL) continue;
        if(symbols[i][0] == '\0') continue;
        if(set_insert(alphabet, &symbols[i])) inserted_count++;     
    }

    return inserted_count;
}


fa_state* fa_state_create(const char* label, bool is_start, bool is_accept){
    if(!label) return NULL;

    fa_state* state = (fa_state*)malloc(sizeof(fa_state));

    if(!state) return NULL;

    state->label = strdup(label);

    state->is_start = is_start;
    state->is_accept = is_accept;
    state->trans = NULL;
    state->ntrans = 0;

    return state;
}

bool fa_trans_create(fa_state *src, fa_state *dest, const char* symbol){

    if(!src || !dest || !symbol) return false;

    fa_trans* new_trans = (fa_trans*)malloc(sizeof(fa_trans));

    if(!new_trans) return false;

    
    strcpy(new_trans->symbol, symbol);

    new_trans->dest = dest;
    new_trans->src = src;

    new_trans->next = src->trans;
    //Insert at beginning
    src->trans = new_trans;
    src->ntrans++;
}

fa_error_t fa_trans_create_validated(const fa_auto *automaton, fa_state *src, 
                               fa_state *dest, const char* symbol){
    
    if(!automaton || !src || !dest || !symbol) return FA_ERR_NULL_ARGUMENT;
    
    

    // if(!set_contains(automaton->alphabet, (void*)symbol)){
    //     return FA_ERR_FA_INVALID_SYMBOL;
    // }
    

    bool src_found = false, dest_found = false;
    for (size_t i = 0; i < automaton->nstates; i++) {
        if (automaton->states[i] == src) src_found = true;
        if (automaton->states[i] == dest) dest_found = true;
        if (src_found && dest_found) break;
    }

    

    if (!src_found || !dest_found) return FA_ERR_FA_STATE_NOT_FOUND;

    if (fa_trans_exists(src, dest, symbol)) {
        return FA_ERR_FA_DUPLICATE_TRANSITION;
    }

    

    fa_trans* new_trans = (fa_trans*)malloc(sizeof(fa_trans));

    if (!new_trans) {
        return FA_ERR_OUT_OF_MEMORY;
    }

    new_trans->symbol = strdup(symbol);
    if (!new_trans->symbol) {
        free(new_trans);
        return FA_ERR_OUT_OF_MEMORY;
    }

    

    new_trans->dest = dest;
    new_trans->src = src;
    new_trans->next = src->trans;
    //Insert at beginning
    src->trans = new_trans;
    src->ntrans++;

    return FA_SUCCESS;
    
}


bool fa_trans_create_epsilon(fa_state *src, fa_state *dest){
    return fa_trans_create(src, dest, FA_EPS_SYMBOL);
}
fa_error_t fa_trans_create_epsilon_validated(const fa_auto *automaton, fa_state *src, fa_state *dest){
    return fa_trans_create_validated(automaton, src, dest, FA_EPS_SYMBOL);
}


fa_state* fa_state_find(const fa_auto* automaton, const char* label){

}

int fa_state_index(const fa_auto* automaton, const fa_state* state){

    if (!automaton || !automaton->states || !automaton->nstates == 0) return -1;
    for (int i = 0; i < automaton->nstates; i++)
    {
        if (strcmp(automaton->states[i]->label, state->label) == 0)
            return i;
    }
    return -1;
}

fa_state* fa_state_get_by_label(const fa_auto* automaton, const char* label) {
    if (!label) {
        return NULL;
    }

    fa_state* state = NULL;

    for (int i = 0; i < automaton->nstates; i++) {
        if (strcmp(automaton->states[i]->label, label) == 0) {
            state = automaton->states[i];
            break;
        }
    }

    return state;
}

fa_state** fa_state_get_dests(fa_state* state, const char* symbol, int nstates){
    if (!state || !symbol) {
        return NULL;
    };

    fa_state** matching_states = malloc(sizeof(fa_state*) * nstates);
    if (!matching_states) {
        return NULL;
    }

    int count = 0;
    fa_trans* current_transition = state->trans;

    while (current_transition) {
        if (strcmp(current_transition->symbol, symbol) == 0) {
            matching_states[count++] = current_transition->dest;
        }
        current_transition = current_transition->next;
    }


    matching_states[count] = NULL;

    if (count == 0) {
        free(matching_states);  // No matching states, free memory
        return NULL;
    }

    fa_state** resized_matching_states = realloc(matching_states, sizeof(fa_state*) * (count + 1));
    if (!resized_matching_states && count > 0) {
        return matching_states;
    }
    return resized_matching_states;
}


fa_state* fa_state_with_max_trans(const fa_auto* automaton){
    if (!automaton || !automaton->states || automaton->nstates == 0) {
        return NULL;
    }

    fa_state* state_with_max_transitions = NULL;
    int max_transitions = -1;

    for (int i = 0; i < automaton->nstates; i++) {
        //number of outgoing transitions
        int current_transitions = automaton->states[i]->ntrans;
        if (current_transitions > max_transitions) {
            max_transitions = current_transitions;
            state_with_max_transitions = automaton->states[i];
        }
    }

    return state_with_max_transitions;
}


// Automaton 
fa_auto* fa_auto_create(int nstates){
    fa_auto* automaton = malloc(sizeof(fa_auto));

    if(!automaton) return NULL;

    automaton->alphabet = set_create(sizeof(char*), compare_strings, hash_string);

    if(!automaton->alphabet){
        free(automaton);
        return NULL;
    } 

    automaton->states = (fa_state**)malloc(nstates * sizeof(fa_state*));

    if (!automaton->states) {
        free(automaton->alphabet);
        free(automaton);
        return NULL;
    }

    for (int i = 0; i < nstates; i++) {
        automaton->states[i] = NULL;
    }

    automaton->nstates = nstates;
    return automaton;
}

fa_auto* fa_auto_from_symbol(char* symbol){
    if (!symbol) {
        return NULL;
    }
    const int num_states = 2;
    Set* alp = set_create(sizeof(char*), compare_strings, hash_string);
    set_insert(alp, strdup(symbol));

    fa_auto* automaton = fa_auto_create(num_states);

    fa_state* q0 = fa_state_create("q0", true, false);
    fa_state* q1 = fa_state_create("q1", false, true);

    fa_trans_create(q0, q1, symbol);

    automaton->states[0] = q0;
    automaton->states[1] = q1;
    automaton->alphabet = alp;

    return automaton;
}

fa_auto* fa_auto_from_regex(const char* regex){
    const char* postfix = infix_to_postfix(regex);
    fa_stack* stack = fa_stack_create(FA_STACK_DEFAULT_CAPACITY);

    for (int i = 0; postfix[i] != '\0'; i++) {
        if (postfix[i] == '*') {
            fa_auto* automaton = fa_stack_pop(stack);
            automaton = fa_auto_kleene(automaton, 0);
            fa_stack_push(stack, automaton);

        }else if (postfix[i] == '?') {
            fa_auto* automaton = fa_stack_pop(stack);
            for (int k = 0; k < automaton->nstates; k++) {
                for (int j = 0; j < automaton->nstates; j++) {
                    if (automaton->states[k]->is_start) {
                        if (automaton->states[j]->is_accept) {
                            fa_trans_create(automaton->states[k], automaton->states[j], "eps");
                        }
                    }
                }
            }
            fa_stack_push(stack, automaton);

        }else if (postfix[i] == '+') {
            fa_auto* automaton = fa_stack_pop(stack);
            automaton = fa_auto_kleene(automaton, 1);
            fa_stack_push(stack, automaton);
        }else if (postfix[i] == '.') {
            const fa_auto *a2 = fa_stack_pop(stack);
            const fa_auto *a1 = fa_stack_pop(stack);

            fa_auto* result = fa_auto_concat(a1, a2);
            if (result) {
                fa_stack_push(stack, result);
            }

        }else if (postfix[i] == '|') {
            const fa_auto *a2 = fa_stack_pop(stack);
            const fa_auto *a1 = fa_stack_pop(stack);

            fa_auto* result = fa_auto_union(a1, a2);
            if (result) {
                fa_stack_push(stack, result);
            }

        }else if (isalnum(postfix[i])) {
            char sym[2] = {postfix[i], '\0'};
            fa_auto* automaton = fa_auto_from_symbol(sym);
            fa_stack_push(stack, automaton);

        }
    }
    return fa_stack_peek(stack);
}


fa_auto* fa_auto_read(const char *filepath){

}
bool fa_trans_exists(fa_state *from, fa_state *to, const char *symbol){
    fa_trans *t = from->trans;
    while (t) {
        if (strcmp(t->dest->label, to->label) == 0 && strcmp(t->symbol, symbol) == 0) {
            return 1;
        }
        t = t->next;
    }
    return 0;
}


fa_state** fa_auto_get_trans_states(const fa_auto* automaton, const char* symbol){
    if (!automaton || !automaton->states || !symbol || automaton->nstates == 0) {
        return NULL;
    }


    fa_state** matching_states = malloc(sizeof(fa_state*) * automaton->nstates);
    if (!matching_states) {
        return NULL;
    }

    int count = 0;
    for (int i = 0; i < automaton->nstates; i++) {
        fa_state* current_state = automaton->states[i];
        if (!current_state) continue;
        fa_trans* transition = current_state->trans;



        while (transition != NULL) {
            if (strcmp(transition->symbol, symbol) == 0) {
                matching_states[count++] = current_state;
                break;
            }
            transition = transition->next;
        }
    }

    matching_states[count] = NULL;

    // State** resized_matching_states = realloc(matching_states, sizeof(State*) * (count + 1));
    // if (!resized_matching_states && count > 0) {
    //     log_message(ERROR, "Failed to resize array of states. Returning original result of matched set.\n");
    //     return matching_states;
    // }
    return matching_states;
}

int fa_auto_has_trans(const fa_auto* automaton, const fa_state* src, 
                      const char* symbol, const fa_state* dest){

    int added = 0;

    for (int i = 0; i < automaton->nstates; i++) {
        fa_state* current_state = automaton->states[i];
        fa_trans* transition = current_state->trans;

        while (transition) {
            fa_state* previous_state = transition->src;
            fa_state* destination_state = transition->dest;
            if (strcmp(transition->symbol, symbol) == 0 && strcmp(previous_state->label, src->label) == 0 && strcmp(destination_state->label, dest->label) == 0) {
                added = 1;
                break;
            }
            transition = transition->next;
        }
    }

    return added;

}


// Automaton Operations
fa_auto* fa_auto_concat(const fa_auto* a1, const fa_auto* a2){
    if(!a1 || !a2) return NULL;

    const int offset = 2;
    fa_auto* automaton = fa_auto_create(a1->nstates + a2->nstates);
    automaton->alphabet = set_union(a1->alphabet, a2->alphabet);

    for (int i = 0; i < a1->nstates; i++) {
        automaton->states[i] = fa_state_create(a1->states[i]->label, a1->states[i]->is_start, false);
    }

    const int last_idx = a1->nstates;
    for (int i = 0; i < a2->nstates; i++) {
        automaton->states[last_idx + i] = fa_state_create(a2->states[i]->label, false, a2->states[i]->is_accept);
    }


    //adding a1 transitions
    for (int i = 0; i < a1->nstates; i++) {
        fa_trans* transition = a1->states[i]->trans;
        while (transition) {
            for (int j = 0; j < a1->nstates; j++) {
                if (strcmp(automaton->states[j]->label, transition->dest->label) == 0) {
                    fa_trans_create(automaton->states[i], automaton->states[j], transition->symbol);
                }
            }

            transition = transition->next;
        }
    }


    //adding a2 transitions
    for (int i = 0; i < a2->nstates; i++) {
        fa_trans* transition = a2->states[i]->trans;
        while (transition) {

            for (int j = 0; j < a2->nstates; j++) {
                if (strcmp(automaton->states[last_idx + j]->label, transition->dest->label) == 0) {
                    fa_trans_create(automaton->states[last_idx + i], automaton->states[last_idx + j], transition->symbol);
                }
            }
            transition = transition->next;
        }
    }

    // adding epsilon transitions
     for (int i = 0; i < a1->nstates; i++) {
         if (a1->states[i]->is_accept) {
             for (int j = 0; j < a2->nstates; j++) {
                 if (a2->states[j]->is_start) {
                     fa_trans_create(automaton->states[i], automaton->states[last_idx + j], "eps");
                 }
             }
         }
    }

    fa_auto_rename_states(automaton);

    return automaton;
}

void fa_auto_rename_states(const fa_auto* automaton){
    if (!automaton) {
        return;
    }
    for (int i = 0; i < automaton->nstates; i++) {
        char* name;
        snprintf(name, sizeof(name), "q%d", i);
        strcpy(automaton->states[i]->label, name);
    }


    for (int i = 0; i < automaton->nstates; i++) {
        fa_trans* transition = automaton->states[i]->trans;
        while (transition) {
            const int dest_index = fa_state_index(automaton, transition->dest);
            const int source_index = fa_state_index(automaton, transition->src);
            if (dest_index != -1 && source_index != -1) {
                transition->src = automaton->states[source_index];
                transition->dest = automaton->states[dest_index];

            }
            transition = transition->next;
        }
    }
}

fa_auto* fa_auto_product(const fa_auto* a1, const fa_auto* a2){
    if (!a1 || !a2) return NULL;

    // Initialisation de l'automate produit
    fa_auto* automaton = fa_auto_create(a1->nstates * a2->nstates);
    if (!automaton) return NULL;

    automaton->alphabet = set_intersection(a1->alphabet, a2->alphabet);

    // Allocation mémoire pour la table des états
    fa_state*** state_automaton = malloc(sizeof(fa_state**) * a1->nstates);
    if (!state_automaton) {
        free(automaton->alphabet);
        free(automaton);
        return NULL;
    }

    for (int i = 0; i < a1->nstates; i++) {
        state_automaton[i] = malloc(sizeof(fa_state*) * a2->nstates);
        if (!state_automaton[i]) {
            for (int k = 0; k < i; k++) free(state_automaton[k]);
            free(state_automaton);
            return NULL;
        }
    }

    // Création des états de l'automate produit
    for (int i = 0; i < a1->nstates; i++) {
        for (int j = 0; j < a2->nstates; j++) {
            char* name_state;
            snprintf(name_state, sizeof(name_state), "%s%s", a1->states[i]->label, a2->states[j]->label);

            int is_starting_state = (a1->states[i]->is_start && a2->states[j]->is_start);
            int is_ending_state = (a1->states[i]->is_accept && a2->states[j]->is_accept);

            state_automaton[i][j] = fa_state_create(name_state, is_starting_state, is_ending_state);
            automaton->states[i * a2->nstates + j] = state_automaton[i][j];
        }
    }

    // Création des transitions
    for (int i = 0; i < a1->nstates; i++) {
        for (int j = 0; j < a2->nstates; j++) {
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

                        int new_index_prev = new_index1_prev * a2->nstates + new_index2_prev;
                        int new_index_dest = new_index1_dest * a2->nstates + new_index2_dest;

                        fa_trans* new_transition_node = malloc(sizeof(fa_trans));
                        if (!new_transition_node) {
                            return NULL;
                        }

                        strcpy(new_transition_node->symbol, transition_node1->symbol);
                        new_transition_node->src = automaton->states[new_index_prev];
                        new_transition_node->dest = automaton->states[new_index_dest];

                        // Vérification de cycle
                        fa_trans* temp = automaton->states[i * a2->nstates + j]->trans;
                        while (temp) {
                            if (temp == new_transition_node) {
                                free(new_transition_node);
                                return NULL;
                            }
                            temp = temp->next;
                        }

                        // Ajout de la transition
                        new_transition_node->next = automaton->states[i * a2->nstates + j]->trans;
                        automaton->states[i * a2->nstates + j]->trans = new_transition_node;

                    }
                    transition_node2 = transition_node2->next;
                }
                transition_node1 = transition_node1->next;
            }
        }
    }

    return automaton;
}

fa_auto* fa_auto_union(const fa_auto* a1, const fa_auto* a2){
    if (!a1 || !a2) return NULL;

    const int offset = 2;
    fa_auto* automaton = fa_auto_create(a1->nstates + a2->nstates + offset);
    automaton->alphabet = set_union(a1->alphabet, a2->alphabet);

    for (int i = 0; i < a1->nstates; i++) {
        automaton->states[i] = fa_state_create(a1->states[i]->label, false, false);
    }

    const int last_idx = a1->nstates;
    for (int i = 0; i < a2->nstates; i++) {
        automaton->states[last_idx + i] = fa_state_create(a2->states[i]->label, false, false);
    }




    //adding a1 transitions
    for (int i = 0; i < a1->nstates; i++) {
        fa_trans* transition = a1->states[i]->trans;
        while (transition) {
            for (int j = 0; j < a1->nstates; j++) {
                if (strcmp(automaton->states[j]->label, transition->dest->label) == 0) {
                    fa_trans_create(automaton->states[i], automaton->states[j], transition->symbol);
                }
            }
            transition = transition->next;
        }
    }



    fa_state *new_origin = fa_state_create("S", true, false);
    fa_state *new_destination = fa_state_create("D", false, true);

    automaton->states[a1->nstates + a2->nstates] = new_origin;
    automaton->states[a1->nstates + a2->nstates + 1] = new_destination;


    //adding a2 transitions
    for (int i = 0; i < a2->nstates; i++) {
        fa_trans* transition = a2->states[i]->trans;
        while (transition) {
            for (int j = 0; j < a2->nstates; j++) {
                if (strcmp(automaton->states[last_idx + j]->label, transition->dest->label) == 0) {
                    fa_trans_create(automaton->states[last_idx + i], automaton->states[last_idx + j], transition->symbol);
                }
            }
            transition = transition->next;
        }
    }

    const int a1_a2_size = a1->nstates + a2->nstates;
    for (int i = 0; i < a1->nstates; i++) {
        if (a1->states[i]->is_start) {
            fa_trans_create(automaton->states[a1_a2_size], automaton->states[i], "eps");
        }
        if (a1->states[i]->is_accept) {
            fa_trans_create(automaton->states[i], automaton->states[a1_a2_size + 1], "eps");
        }
    }


    for (int i = 0; i < a2->nstates; i++) {
        if (a2->states[i]->is_start) {
            fa_trans_create(automaton->states[a1_a2_size], automaton->states[last_idx + i], "eps");
        }
        if (a2->states[i]->is_accept) {
            fa_trans_create(automaton->states[last_idx + i], automaton->states[a1_a2_size + 1], "eps");
        }
    }

    fa_auto_rename_states(automaton);

    return automaton;
}

fa_auto* fa_auto_kleene(fa_auto* automaton, int variant){
     if (!automaton) {
        return NULL;
    }

    fa_auto* new_aut = NULL;

    if (variant == 0) {
        const int offset = 2;
        new_aut = fa_auto_create(automaton->nstates + offset);
        new_aut->alphabet = automaton->alphabet;

        fa_state *new_origin = fa_state_create("S", true, false);
        fa_state *new_destination = fa_state_create("D", false, true);

        for (int i = 0; i < automaton->nstates; i++) {
            new_aut->states[i] = automaton->states[i];
        }

        new_aut->states[automaton->nstates] = new_origin;
        new_aut->states[automaton->nstates + 1] = new_destination;

        for (int i = 0; i < automaton->nstates; i++) {
            for (int j = 0; j < automaton->nstates; j++) {
                if (automaton->states[i]->is_start && automaton->states[j]->is_accept) {
                    fa_trans_create(automaton->states[j], automaton->states[i], "eps");
                }
            }
        }


        for (int i = 0; i < automaton->nstates; i++) {
            if (automaton->states[i]->is_start) {
                fa_trans_create(automaton->states[automaton->nstates], automaton->states[i], "eps");
            }

            if (automaton->states[i]->is_accept) {
                fa_trans_create(automaton->states[i], automaton->states[automaton->nstates + 1], "eps");
            }
        }

        fa_trans_create(automaton->states[automaton->nstates], automaton->states[automaton->nstates + 1], "eps");


        for (int i = 0; i < automaton->nstates; i++) {
            automaton->states[i]->is_accept = false;
            automaton->states[i]->is_start = false;
        }



    }else {
        new_aut = fa_auto_create(automaton->nstates);
        new_aut->alphabet = automaton->alphabet;
        for (int i = 0; i < automaton->nstates; i++) {
            new_aut->states[i] = automaton->states[i];
        }

        for (int i = 0; i < automaton->nstates; i++) {
            for (int j = 0; j < automaton->nstates; j++) {
                if (automaton->states[i]->is_start && automaton->states[j]->is_accept) {
                    fa_trans_create(automaton->states[j], automaton->states[i], "eps");
                }
            }
        }

    }

    return new_aut;
}


fa_auto* fa_auto_to_dfa(fa_auto *nfa){
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

fa_auto* fa_auto_optimize(fa_auto* automaton){
    if (!automaton) {
        return NULL;
    }


    const fa_auto* det = fa_auto_to_dfa(automaton);
    fa_auto* minim = fa_auto_minimize_moore(det);
    return minim;
}


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

    int groups[automaton->nstates];

    for (int i = 0; i < automaton->nstates; i++) {
        groups[i] = -1;
    }

    int state_idx = 0;

    HashTable* rpr_to_group = hash_table_create_string_int(automaton->nstates);
    HashTable* group_to_rpr_minim = hash_table_create_int_string(num_states);
    

    for (int j = 0; j < automaton->nstates; j++) {
        fa_state* current_state = automaton->states[j];
        int current_state_group = final_partition[fa_state_index(automaton, current_state)];
        if (!exists_in_array(groups, automaton->nstates, current_state_group)) {
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

    for (int i = 0; i < automaton->nstates; i++) {
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
    for (int i = 0; i < automaton->nstates; i++) {
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


fa_auto* fa_auto_minimize_moore(const fa_auto *automaton){
    const int N = automaton->nstates;
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

void fa_state_destroy(fa_state* s){

}

void fa_auto_destroy(fa_auto* a) {
    if (!a) return;
    
    
    for (int i = 0; i < a->nstates; i++) {
        fa_state* state = a->states[i];
        if (!state) continue;
        
        
        fa_trans* current = state->trans;
        while (current) {
            fa_trans* next = current->next;
            free(current->symbol);  
            free(current);
            current = next;
        }
        
        free(state->label);  
        free(state);
    }
    
    
    if (a->alphabet) {
        set_destroy(a->alphabet);  
    }
    
    free(a->states);
    free(a);
}

// Stack Operations

fa_stack* fa_stack_create(int initial_capacity) {
    fa_stack *stack = malloc(sizeof(fa_stack));
    if (!stack) return NULL;
    
    if (!fa_stack_init(stack, initial_capacity)) {
        free(stack);
        return NULL;
    }
    return stack;
}

bool fa_stack_init(fa_stack *stack, int initial_capacity) {
    if (initial_capacity <= 0) {
        initial_capacity = FA_STACK_DEFAULT_CAPACITY;
    }
    
    stack->items = malloc(sizeof(fa_auto*) * initial_capacity);
    if (!stack->items) return false;
    
    stack->top = -1;
    stack->capacity = initial_capacity;
    return true;
}

bool fa_stack_push(fa_stack *stack, fa_auto *automaton) {
    if (!stack || !automaton) return false;
    if (stack->top + 1 >= stack->capacity) {
        int new_capacity = stack->capacity * 2;
        fa_auto **new_items = realloc(stack->items, 
                                      sizeof(fa_auto*) * new_capacity);
        if (!new_items) return false;
        
        stack->items = new_items;
        stack->capacity = new_capacity;
    }
    
    stack->items[++stack->top] = automaton;
    return true;
}


fa_auto* fa_stack_pop(fa_stack *stack) {
    if (fa_stack_is_empty(stack)) {
        return NULL;
    }
    return stack->items[stack->top--];
}

fa_auto* fa_stack_peek(const fa_stack *stack) {
    if (fa_stack_is_empty(stack)) {
        return NULL;
    }
    return stack->items[stack->top];
}

bool fa_stack_is_empty(const fa_stack *stack) {
    return stack->top == -1;
}

void fa_stack_clear(fa_stack *stack) {
    stack->top = -1;
}

void fa_stack_destroy(fa_stack *stack, bool free_automata) {
    if (!stack) return;
    
    if (free_automata) {
        for (int i = 0; i <= stack->top; i++) {
            fa_auto_destroy(stack->items[i]);
        }
    }
    
    free(stack->items);
    stack->items = NULL;
    stack->top = -1;
    stack->capacity = 0;
}

int fa_stack_size(const fa_stack *stack) {
    return stack->top + 1;
}

bool fa_stack_is_full(const fa_stack *stack) {
    // the stack is never truly "full"
    // It can always grow (until system memory is exhausted)
    return false;
}



bool fa_auto_accepts(const fa_auto* automaton, const char* word){
    if (!automaton || !word || !automaton->states) return false;

    //0 -> word rejected
    //1 -> word accepted

    for (int i = 0; i < automaton->nstates; i++) {
        fa_state* state = automaton->states[i];

        if (state->is_start) {
            fa_state* current_state = state;
            int symbol_idx = 0;

            while (1) {
                fa_trans* current_transition = current_state->trans;
                int found = 0;


                char symbol[2] = {word[symbol_idx], '\0'};

                while (current_transition) {
                    if (strcmp(current_transition->symbol, "eps") == 0) {

                        current_state = current_transition->dest;
                        found = 1;
                        break;
                    }
                    else if (strcmp(current_transition->symbol, symbol) == 0) {
                        current_state = current_transition->dest;
                        found = 1;
                        symbol_idx++;
                        break;
                    }
                    current_transition = current_transition->next;
                }

                if (!found) {
                    break;
                }


                if (word[symbol_idx] == '\0') {
                    if (current_state->is_accept) {
                        return 1;
                    }
                    //Checking for epsilon transitions after the stopping state
                    int epsilon_found;
                    do {
                        epsilon_found = 0;
                        fa_trans* epsilon_transition = current_state->trans;

                        while (epsilon_transition) {
                            if (strcmp(epsilon_transition->symbol, "eps") == 0) {
                                
                                current_state = epsilon_transition->dest;


                                if (current_state->is_accept) {
                                    return 1;
                                }
                                epsilon_found = 1;
                            }
                            epsilon_transition = epsilon_transition->next;
                        }
                    } while (epsilon_found);
                    break;
                }
            }
        }
    }

    return 0;
}