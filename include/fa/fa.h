// include/fa/fa.h
#ifndef FA_FA_H
#define FA_FA_H
#include "../set/set.h"
#include "../fa_error.h"
#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct fa_state fa_state;
typedef struct fa_trans fa_trans;
typedef struct fa_auto fa_auto;


/**
 * @brief Represents a single state in a finite automaton.
 *
 * Each state maintains a linked list of outgoing transitions and flags
 * indicating whether it is a start or accept state.
 */
typedef struct fa_state {
    char *label;              /**< Human-readable state identifier */
    bool is_start;             /**< Non-zero if this is a start state */
    bool is_accept;            /**< Non-zero if this is an accept state */
    struct fa_trans *trans;   /**< Head of transition list */
    int ntrans;               /**< Number of outgoing transitions */
} fa_state;


/**
 * @brief A dynamic stack for storing automata.
 *
 * Used primarily during regex parsing and Thompson's construction algorithm.
 * The stack grows dynamically as needed and can hold an arbitrary number
 * of automata.
 */
typedef struct fa_stack {
    fa_auto **items;          /**< Dynamic array of automaton pointers */
    int top;                  /**< Index of top element (-1 if empty) */
    int capacity;             /**< Current allocated capacity */
} fa_stack;


/**
 * @brief Represents a transition between states in a finite automaton.
 *
 * Transitions form a linked list attached to each state. Each transition
 * specifies an input symbol and a destination state.
 */
typedef struct fa_trans {
    char *symbol;             /**< Input symbol triggering this transition */
    fa_state *dest;           /**< Destination state */
    fa_state *src;            /**< Source state (for reverse lookups) */
    struct fa_trans *next;    /**< Next transition in the list */
} fa_trans;


/**
 * @brief Represents a finite automaton.
 *
 * A finite automaton consists of a set of states, an input alphabet,
 * and transitions between states. The automaton can be either deterministic
 * (DFA) or nondeterministic (NFA) depending on the transition structure.
 */
typedef struct fa_auto {
    int nstates;              /**< Number of states in the automaton */
    Set *alphabet;            /**< Input alphabet (set of symbols) */
    fa_state **states;        /**< Array of pointers to states */
} fa_auto;


// ============================================================================
// Automaton Alphabet operations
// ============================================================================

/**
 * Adds valid alphabet symbols to a finite automaton's alphabet set.
 * Skips NULL pointers and empty strings, as neither are valid alphabet symbols.
 * 
 * @param alphabet Set to store alphabet symbols
 * @param symbols Array of string pointers representing potential symbols
 * @param count Number of elements in the symbols array
 * @return Number of valid symbols successfully added to the alphabet
 */

size_t fa_alphabet_insert_symbols(Set* alphabet, const char* symbols[], size_t count);


// ============================================================================
// State and Transition Operations
// ============================================================================

/**
 * @brief Creates a new state with the specified properties.
 * @param label Human-readable state identifier
 * @param is_start Non-zero if this is a start state
 * @param is_accept Non-zero if this is an accept state
 * @return Pointer to the newly created state, or NULL on failure
 */
fa_state* fa_state_create(const char* label, bool is_start, bool is_accept);

/**
 * @brief Creates a transition between two states.
 * @param src Source state
 * @param dest Destination state
 * @param symbol Input symbol triggering the transition
 * @return true if transition was created successfully, false otherwise
 */
bool fa_trans_create(fa_state *src, fa_state *dest, const char* symbol);
fa_error_t fa_trans_create_validated(const fa_auto *automaton, fa_state *src, 
                               fa_state *dest, const char* symbol);


bool fa_trans_create_epsilon(fa_state *src, fa_state *dest);
fa_error_t fa_trans_create_epsilon_validated(const fa_auto *automaton, fa_state *src, fa_state *dest);

fa_state* fa_state_get_by_label(const fa_auto* automaton, const char* label);

/**
 * @brief Finds a state in an automaton by its label.
 * @param automaton The automaton to search
 * @param label The state label to find
 * @return Pointer to the state, or NULL if not found
 */
fa_state* fa_state_find(const fa_auto* automaton, const char* label);

/**
 * @brief Gets the index of a state in the automaton's state array.
 * @param automaton The automaton
 * @param state The state to find
 * @return Index of the state, or -1 if not found
 */
int fa_state_index(const fa_auto* automaton, const fa_state* state);

/**
 * @brief Finds the state with the most outgoing transitions.
 * @param automaton The automaton to search
 * @return Pointer to the state with maximum transitions
 */
fa_state* fa_state_with_max_trans(const fa_auto* automaton);

/**
 * @brief Gets all destination states reachable via a specific symbol from a state.
 * @param state Source state
 * @param symbol Transition symbol
 * @param nstates Total number of states in automaton (for array allocation)
 * @return Array of destination states (caller must free with fa_state_free_array)
 */
fa_state** fa_state_get_dests(fa_state* state, const char* symbol, int nstates);



// ============================================================================
// Automaton Creation and Initialization
// ============================================================================

/**
 * @brief Creates a new automaton with the specified number of states.
 * @param nstates Number of states to allocate
 * @return Pointer to the newly created automaton, or NULL on failure
 */
fa_auto* fa_auto_create(int nstates);

/**
 * @brief Creates an automaton from a single symbol.
 * @param symbol The input symbol
 * @return Simple automaton accepting only the given symbol
 */
fa_auto* fa_auto_from_symbol(char* symbol);

/**
 * @brief Constructs an automaton from a regular expression.
 * @param regex Regular expression string
 * @return Automaton accepting the language defined by the regex
 */
fa_auto* fa_auto_from_regex(const char* regex);

/**
 * @brief Reads an automaton from a file.
 * @param filepath Path to the automaton definition file
 * @return Loaded automaton, or NULL on failure
 */
fa_auto* fa_auto_read(const char *filepath);

// ============================================================================
// Automaton Conversion and Generation
// ============================================================================

/**
 * @brief Converts an NFA to a DFA using subset construction.
 * @param nfa The nondeterministic finite automaton
 * @return Equivalent deterministic finite automaton
 */
fa_auto* fa_auto_to_dfa(fa_auto *nfa);

/**
 * @brief Removes epsilon transitions from an automaton.
 * @param automaton The automaton to modify (modified in place)
 */
void fa_auto_remove_epsilon(fa_auto *automaton);

/**
 * @brief Computes the epsilon closure of a state.
 * @param state Starting state
 * @param closure Array to store closure states (output)
 * @param size Pointer to store the number of states in closure (output)
 */
void fa_auto_epsilon_closure(fa_state *state, fa_state **closure, int *size);

/**
 * @brief Adds transitions via epsilon closure computation.
 * @param automaton The automaton to process
 */
void fa_auto_add_epsilon_trans(fa_auto *automaton);

/**
 * @brief Updates final/accept states after transformation.
 * @param automaton The automaton to update
 */
void fa_auto_update_final(fa_auto *automaton);

/**
 * @brief Renames states and transitions with canonical labels.
 * @param automaton The automaton to rename
 */
void fa_auto_rename_states(const fa_auto* automaton);

// ============================================================================
// Automaton Operations
// ============================================================================

/**
 * @brief Concatenates two automata (L1 · L2).
 * @param a1 First automaton
 * @param a2 Second automaton
 * @return New automaton accepting L(a1) · L(a2)
 */
fa_auto* fa_auto_concat(const fa_auto* a1, const fa_auto* a2);

/**
 * @brief Computes the product (intersection) of two automata.
 * @param a1 First automaton
 * @param a2 Second automaton
 * @return New automaton accepting L(a1) ∩ L(a2)
 */
fa_auto* fa_auto_product(const fa_auto* a1, const fa_auto* a2);

/**
 * @brief Computes the union of two automata (L1 ∪ L2).
 * @param a1 First automaton
 * @param a2 Second automaton
 * @return New automaton accepting L(a1) ∪ L(a2)
 */
fa_auto* fa_auto_union(const fa_auto* a1, const fa_auto* a2);

/**
 * @brief Applies Kleene star operation to an automaton (L*).
 * @param automaton Input automaton
 * @param variant Implementation variant to use
 * @return New automaton accepting L(automaton)*
 */
fa_auto* fa_auto_kleene(fa_auto* automaton, int variant);

/**
 * @brief Optimizes an automaton (minimizes states).
 * @param automaton The automaton to optimize
 * @return Optimized automaton with minimal states
 */
fa_auto* fa_auto_optimize(fa_auto* automaton);

/**
 * @brief Minimizes an automaton using Moore's algorithm.
 * @param automaton The automaton to minimize
 * @return Minimized automaton
 */
fa_auto* fa_auto_minimize_moore(const fa_auto *automaton);

// ============================================================================
// Automaton Queries and Checks
// ============================================================================

/**
 * @brief Checks if a transition exists between two states.
 * @param from Source state
 * @param to Destination state
 * @param symbol Transition symbol
 * @return Non-zero if transition exists, 0 otherwise
 */
bool fa_trans_exists(fa_state *from, fa_state *to, const char *symbol);

/**
 * @brief Checks if a specific transition has been added to an automaton.
 * @param automaton The automaton
 * @param src Source state
 * @param symbol Transition symbol
 * @param dest Destination state
 * @return Non-zero if transition is already present, 0 otherwise
 */
int fa_auto_has_trans(const fa_auto* automaton, const fa_state* src, 
                      const char* symbol, const fa_state* dest);

/**
 * @brief Checks if an array of states contains a specific state.
 * @param states Array of states
 * @param size Size of the array
 * @param state State to search for
 * @return Non-zero if state is in array, 0 otherwise
 */
int fa_state_contains(fa_state **states, int size, fa_state *state);



/**
 * @brief Gets all states that have outgoing transitions on a given symbol.
 * @param automaton The automaton
 * @param symbol The transition symbol
 * @return Array of states with transitions on symbol
 */
fa_state** fa_auto_get_trans_states(const fa_auto* automaton, const char* symbol);

/**
 * @brief Simulates the automaton on an input word.
 * @param automaton The automaton
 * @param word Input word to process
 * @return Non-zero if word is accepted, 0 otherwise
 */
bool fa_auto_accepts(const fa_auto* automaton, const char* word);

// ============================================================================
// Output and Debugging
// ============================================================================




void fa_state_destroy(fa_state* s);
void fa_auto_destroy(fa_auto* a);


// ============================================================================
// Stack Operations (for regex parsing/Thompson construction)
// ============================================================================

/**
 * @brief Creates a new automaton stack with initial capacity.
 * @param initial_capacity Initial number of slots (use 0 for default)
 * @return Pointer to the newly created stack, or NULL on failure
 */
fa_stack* fa_stack_create(int initial_capacity);

/**
 * @brief Initializes an automaton stack (for stack-allocated structures).
 * @param stack Stack structure to initialize
 * @param initial_capacity Initial number of slots (use 0 for default)
 * @return true on success, false on failure
 */
bool fa_stack_init(fa_stack *stack, int initial_capacity);

/**
 * @brief Pushes an automaton onto the stack.
 * @param stack The stack
 * @param automaton Automaton to push
 */
bool fa_stack_push(fa_stack *stack, fa_auto *automaton);

/**
 * @brief Pops an automaton from the stack.
 * @param stack The stack
 * @return Popped automaton, or NULL if stack is empty
 */
fa_auto* fa_stack_pop(fa_stack *stack);

/**
 * @brief Peeks at the top automaton without removing it.
 * @param stack The stack
 * @return Top automaton, or NULL if stack is empty
 */
fa_auto* fa_stack_peek(const fa_stack *stack);

/**
 * @brief Checks if the stack is empty.
 * @param stack The stack
 * @return true if empty, false otherwise
 */
bool fa_stack_is_empty(const fa_stack *stack);

/**
 * @brief Checks if the stack is full.
 * @param stack The stack
 * @return true if full, false otherwise
 */
bool fa_stack_is_full(const fa_stack *stack);


/**
 * @brief Gets the number of automata currently on the stack.
 * @param stack The stack
 * @return Number of elements
 */
int fa_stack_size(const fa_stack *stack);

/**
 * @brief Clears all automata from the stack without freeing them.
 * @param stack The stack to clear
 */
void fa_stack_clear(fa_stack *stack);

/**
 * @brief Frees the stack and optionally its contained automata.
 * @param stack The stack to free
 * @param free_automata If true, also frees all automata on the stack
 */
void fa_stack_destroy(fa_stack *stack, bool free_automata);


#ifdef __cplusplus
}
#endif

#endif