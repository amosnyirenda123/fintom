#include "../include/fa/fa.h"
#include "../include/set/set.h"
#include "../include/common.h"
#include "../include/hash/hash_table.h"
#include "../include/regex/regexpr.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define FA_STACK_DEFAULT_CAPACITY 16


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

    if (!state->label) {
        free(state);
        return NULL;
    }

    state->is_start = is_start;
    state->is_accept = is_accept;
    state->trans = NULL;
    state->ntrans = 0;

    return state;
}


fa_error_t fa_auto_create_state(fa_auto* automaton, const char* label, bool is_start, bool is_accept){
    if(!automaton || !label) return FA_ERR_NULL_ARGUMENT;

    if (automaton->nstates >= automaton->capacity) return FA_ERR_BUFFER_TOO_SMALL;

    
    if(fa_state_get_by_label(automaton, label)) return FA_ERR_FA_DUPLICATE_STATE;

    fa_state* state = (fa_state*)malloc(sizeof(fa_state));

    if(!state) return FA_ERR_OUT_OF_MEMORY;

    state->label = strdup(label);
    if (!state->label) {
        free(state);
        return FA_ERR_OUT_OF_MEMORY;
    }

    state->is_start = is_start;
    state->is_accept = is_accept;
    state->trans = NULL;
    state->ntrans = 0;

    automaton->states[automaton->nstates++] = state;

    return FA_SUCCESS;

}

fa_error_t fa_trans_create(fa_state *src, fa_state *dest, const char* symbol){

    if(!src || !dest || !symbol) return FA_ERR_NULL_ARGUMENT;

    fa_trans* new_trans = (fa_trans*)malloc(sizeof(fa_trans));

    if(!new_trans) return FA_ERR_OUT_OF_MEMORY;

    
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

fa_error_t fa_auto_create_trans(const fa_auto *automaton, fa_state *src, 
                               fa_state *dest, const char* symbol){
    
    if(!automaton || !src || !dest || !symbol) return FA_ERR_NULL_ARGUMENT;
    
    

    if(!set_contains(automaton->alphabet, &symbol)){
        return FA_ERR_FA_INVALID_SYMBOL;
    }
    

    bool src_found = false, dest_found = false;
    for (size_t i = 0; i < automaton->capacity; i++) {
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
fa_error_t fa_auto_create_epsilon_trans(const fa_auto *automaton, fa_state *src, fa_state *dest){
    return fa_auto_create_trans(automaton, src, dest, FA_EPS_SYMBOL);
}


fa_state* fa_state_find(const fa_auto* automaton, const char* label){

}

int fa_state_index(const fa_auto* automaton, const fa_state* state){

    if (!automaton || !automaton->states || !automaton->capacity == 0) return -1;
    for (int i = 0; i < automaton->capacity; i++)
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

    for (int i = 0; i < automaton->capacity; i++) {
        if (strcmp(automaton->states[i]->label, label) == 0) {
            state = automaton->states[i];
            break;
        }
    }

    return state;
}

fa_state** fa_state_get_dests(fa_state* state, const char* symbol, int capacity){
    if (!state || !symbol) {
        return NULL;
    };

    fa_state** matching_states = malloc(sizeof(fa_state*) * capacity);
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
    if (!automaton || !automaton->states || automaton->capacity == 0) {
        return NULL;
    }

    fa_state* state_with_max_transitions = NULL;
    int max_transitions = -1;

    for (int i = 0; i < automaton->capacity; i++) {
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
fa_auto* fa_auto_create(int capacity){
    fa_auto* automaton = malloc(sizeof(fa_auto));

    if(!automaton) return NULL;

    automaton->alphabet = set_create(sizeof(char*), compare_strings, hash_string);

    if(!automaton->alphabet){
        free(automaton);
        return NULL;
    } 

    automaton->states = (fa_state**)malloc(capacity * sizeof(fa_state*));

    if (!automaton->states) {
        free(automaton->alphabet);
        free(automaton);
        return NULL;
    }

    for (int i = 0; i < capacity; i++) {
        automaton->states[i] = NULL;
    }

    automaton->capacity = capacity;
    automaton->nstates = 0;
    return automaton;
}

fa_auto* fa_auto_from_symbol(char* symbol) {
    if (!symbol) {
        return NULL;
    }
    
    const int num_states = 2;
    Set* alp = NULL;
    char* alpha_symbol = NULL;
    fa_auto* automaton = NULL;
    fa_state* q0 = NULL;
    fa_state* q1 = NULL;
    
    // Initialize alphabet
    alp = set_create(sizeof(char*), compare_strings, hash_string);
    if (!alp) {
        goto cleanup;
    }
    
    alpha_symbol = strdup(symbol);
    if (!alpha_symbol) {
        goto cleanup;
    }
    
    if (!set_insert(alp, &alpha_symbol)) {
        goto cleanup;
    }
    
    
    automaton = fa_auto_create(num_states);
    if (!automaton) {
        goto cleanup;
    }
    
    q0 = fa_state_create("q0", true, false);
    if (!q0) {
        goto cleanup;
    }
    
    q1 = fa_state_create("q1", false, true);
    if (!q1) {
        goto cleanup;
    }
    
    fa_error_t success = fa_trans_create(q0, q1, symbol);
    if (success != FA_SUCCESS) {
        goto cleanup;
    }
    
    
    automaton->states[0] = q0;
    automaton->states[1] = q1;
    automaton->nstates = num_states;
    automaton->alphabet = alp;
    
    
    
    return automaton;

cleanup:
    
    free(alpha_symbol);  
    if (alp) set_destroy(alp);
    if (q0) fa_state_destroy(q0);
    if (q1) fa_state_destroy(q1);
    if (automaton) fa_auto_destroy(automaton);
    return NULL;
}

fa_auto* fa_auto_from_regex(const char* regex){
    // const char* postfix = infix_to_postfix(regex);
    // fa_stack* stack = fa_stack_create(FA_STACK_DEFAULT_CAPACITY);

    // for (int i = 0; postfix[i] != '\0'; i++) {
    //     if (postfix[i] == '*') {
    //         fa_auto* automaton = fa_stack_pop(stack);
    //         automaton = fa_auto_kleene(automaton, 0);
    //         fa_stack_push(stack, automaton);

    //     }else if (postfix[i] == '?') {
    //         fa_auto* automaton = fa_stack_pop(stack);
    //         for (int k = 0; k < automaton->capacity; k++) {
    //             for (int j = 0; j < automaton->capacity; j++) {
    //                 if (automaton->states[k]->is_start) {
    //                     if (automaton->states[j]->is_accept) {
    //                         fa_trans_create(automaton->states[k], automaton->states[j], "eps");
    //                     }
    //                 }
    //             }
    //         }
    //         fa_stack_push(stack, automaton);

    //     }else if (postfix[i] == '+') {
    //         fa_auto* automaton = fa_stack_pop(stack);
    //         automaton = fa_auto_kleene(automaton, 1);
    //         fa_stack_push(stack, automaton);
    //     }else if (postfix[i] == '.') {
    //         const fa_auto *a2 = fa_stack_pop(stack);
    //         const fa_auto *a1 = fa_stack_pop(stack);

    //         fa_auto* result = fa_auto_concat(a1, a2);
    //         if (result) {
    //             fa_stack_push(stack, result);
    //         }

    //     }else if (postfix[i] == '|') {
    //         const fa_auto *a2 = fa_stack_pop(stack);
    //         const fa_auto *a1 = fa_stack_pop(stack);

    //         fa_auto* result = fa_auto_union(a1, a2);
    //         if (result) {
    //             fa_stack_push(stack, result);
    //         }

    //     }else if (isalnum(postfix[i])) {
    //         char sym[2] = {postfix[i], '\0'};
    //         fa_auto* automaton = fa_auto_from_symbol(sym);
    //         fa_stack_push(stack, automaton);

    //     }
    // }
    // return fa_stack_peek(stack);
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
    if (!automaton || !automaton->states || !symbol || automaton->capacity == 0) {
        return NULL;
    }


    fa_state** matching_states = malloc(sizeof(fa_state*) * automaton->capacity);
    if (!matching_states) {
        return NULL;
    }

    int count = 0;
    for (int i = 0; i < automaton->capacity; i++) {
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

    for (int i = 0; i < automaton->capacity; i++) {
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

void fa_auto_rename_states(const fa_auto* automaton){
    if (!automaton) {
        return;
    }
    
     for (int i = 0; i < automaton->capacity; i++) {
        fa_state* state = automaton->states[i];
        if (!state) continue;
        
        char* old_label = state->label;
        char new_name[32];
        snprintf(new_name, sizeof(new_name), "q%d", i);
        
        
        char* new_label = strdup(new_name);
        if (new_label) {
            if (old_label) {
                free(old_label);
            }
            state->label = new_label;
        } else {
            state->label = old_label;
        }

        
    }
}












void fa_state_destroy(fa_state* s){

}

void fa_auto_destroy(fa_auto* a) {
    if (!a) return;
    
    
    for (int i = 0; i < a->capacity; i++) {
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

    for (int i = 0; i < automaton->capacity; i++) {
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