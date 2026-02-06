#ifndef FA_OPERATIONS_H
#define FA_OPERATIONS_H

#include "fa.h"
#include <stdarg.h>




/**
 * Determinization algorithm types for finite automata
 * (Converting NFA/ε-NFA to DFA)
 */

 /**
 * ALGORITHM DESCRIPTIONS:
 * 
 * 1. FA_DETERMINIZE_SUBSET: Standard subset construction
 *    - Generates all possible subsets of NFA states
 *    - Complete but can generate 2^n states
 *    - Good for small NFAs or when complete DFA is needed
 * 
 * 2. FA_DETERMINIZE_LAZY: Lazy subset construction
 *    - Only generates reachable subsets
 *    - More memory efficient for sparse NFAs
 *    - May be slower for dense NFAs
 * 
 * 3. FA_DETERMINIZE_BFS: Breadth-first construction
 *    - Explores states level by level
 *    - Good for finding shortest paths
 *    - Can be combined with early termination
 * 
 * 4. FA_DETERMINIZE_DFS: Depth-first construction
 *    - Explores one branch fully before others
 *    - Lower memory usage for deep NFAs
 *    - May find smaller DFAs in practice
 * 
 * 5. FA_DETERMINIZE_INCREMENTAL: Incremental construction
 *    - Builds DFA incrementally as needed
 *    - Good for on-the-fly applications
 *    - Can be combined with caching
 */
typedef enum {
    FA_DETERMINIZE_NONE         = 0x00,     // No determinization
    FA_DETERMINIZE_SUBSET       = 0x01,     // Standard subset construction
    FA_DETERMINIZE_LAZY         = 0x02,     // Lazy/eager subset construction
    FA_DETERMINIZE_BFS          = 0x04,     // Breadth-first subset construction
    FA_DETERMINIZE_DFS          = 0x08,     // Depth-first subset construction
    FA_DETERMINIZE_INCREMENTAL  = 0x10,     // Incremental determinization
    
    // Algorithm options and optimizations
    FA_DETERMINIZE_MINIMIZE     = 0x0100,   // Minimize during/after construction
    FA_DETERMINIZE_REMOVE_EPS   = 0x0200,   // Remove ε-transitions first
    FA_DETERMINIZE_OPTIMIZE     = 0x0400,   // Apply optimizations during construction
    FA_DETERMINIZE_CACHE        = 0x0800,   // Cache subset computations
    FA_DETERMINIZE_VERBOSE      = 0x1000,   // Print debugging information
    FA_DETERMINIZE_KEEP_NAMES   = 0x2000,   // Generate meaningful state names
    FA_DETERMINIZE_IN_PLACE     = 0x4000,   // Try to determinize in-place if possible
    
    // State naming conventions for subset states
    FA_DETERMINIZE_NAMES_BITSET = 0x00010000, // Names like "{0,2,3}"
    FA_DETERMINIZE_NAMES_SIMPLE = 0x00020000, // Names like "S0", "S1", ...
    FA_DETERMINIZE_NAMES_ORIG   = 0x00040000, // Names like "q0+q2+q3"
    
    // Common combinations
    FA_DETERMINIZE_DEFAULT      = FA_DETERMINIZE_SUBSET,
    FA_DETERMINIZE_FAST         = FA_DETERMINIZE_LAZY | FA_DETERMINIZE_CACHE,
    FA_DETERMINIZE_SIMPLE       = FA_DETERMINIZE_SUBSET | FA_DETERMINIZE_REMOVE_EPS,
    FA_DETERMINIZE_COMPLETE     = FA_DETERMINIZE_SUBSET | FA_DETERMINIZE_MINIMIZE | FA_DETERMINIZE_REMOVE_EPS,
    
    // Algorithm mask (bits 0-15 for algorithm selection)
    FA_DETERMINIZE_ALGO_MASK    = 0x00FF,
} fa_determinize_algorithm;


/**
 * Minimization algorithm types for finite automata
 */
typedef enum {
    FA_MINIMIZE_NONE        = 0x00,     // No minimization
    FA_MINIMIZE_HOPCROFT    = 0x01,     // Hopcroft's algorithm (O(n log n))
    FA_MINIMIZE_MOORE       = 0x02,     // Moore's algorithm (O(n²))
    FA_MINIMIZE_BRZOZOWSKI  = 0x04,     // Brzozowski's algorithm (reverse, determinize, repeat)
    FA_MINIMIZE_TABLE       = 0x08,     // Table-filling algorithm (Myhill-Nerode)
    
    // Algorithm variants or options
    FA_MINIMIZE_KEEP_NAMES  = 0x10,     // Preserve original state names if possible
    FA_MINIMIZE_VERBOSE     = 0x20,     // Print debugging information
    FA_MINIMIZE_IN_PLACE    = 0x40,     // Minimize in-place instead of creating new automaton
    
    // Common combinations
    FA_MINIMIZE_DEFAULT     = FA_MINIMIZE_HOPCROFT,
    FA_MINIMIZE_FAST        = FA_MINIMIZE_HOPCROFT,
    FA_MINIMIZE_SIMPLE      = FA_MINIMIZE_TABLE,
    FA_MINIMIZE_THEORETIC   = FA_MINIMIZE_BRZOZOWSKI,
} fa_minimize_algorithm;


typedef enum {
    FA_KLEENE_STAR = 0x01,    // Zero or more
    FA_KLEENE_PLUS = 0x02     // One or more
} fa_kleene_type;

// Operation flags (can be combined using bitwise OR)
typedef enum {
    FA_OP_UNION         = 1 << 0,
    FA_OP_INTERSECTION  = 1 << 1,
    FA_OP_DIFFERENCE    = 1 << 2,    // A - B
    FA_OP_SYMMETRIC_DIFF = 1 << 3,   // (A ∪ B) - (A ∩ B)
    FA_OP_CONCATENATION = 1 << 4,
    FA_OP_COMPLEMENT    = 1 << 5,    // Only uses A
    FA_OP_REVERSE       = 1 << 6,    // Only uses A
    FA_OP_KLEENE_STAR   = 1 << 7,    // Only uses A
    FA_OP_MINIMIZE      = 1 << 8,    // Only uses A
    FA_OP_DETERMINIZE   = 1 << 9,    // Only uses A
    
    // Shorthand combinations
    FA_OP_ALL_BINARY    = FA_OP_UNION | FA_OP_INTERSECTION | 
                          FA_OP_DIFFERENCE | FA_OP_SYMMETRIC_DIFF | 
                          FA_OP_CONCATENATION,
    FA_OP_ALL_UNARY     = FA_OP_COMPLEMENT | FA_OP_REVERSE | 
                          FA_OP_KLEENE_STAR | FA_OP_MINIMIZE | 
                          FA_OP_DETERMINIZE,
    FA_OP_ALL           = FA_OP_ALL_BINARY | FA_OP_ALL_UNARY
} fa_operation_flags;

typedef enum {
    FA_OP_LEFT_ASSOCIATIVE  = 1 << 16,  // Default: ((A op B) op C)
    FA_OP_RIGHT_ASSOCIATIVE = 1 << 17,  // (A op (B op C))
    FA_OP_PARALLEL          = 1 << 18,  // Try to compose in parallel when possible
} fa_composition_mode;

// Result structure containing all requested operations
typedef struct fa_operation_results {
    fa_auto* union_result;
    fa_auto* intersection_result;
    fa_auto* difference_result;        // A - B
    fa_auto* symmetric_difference_result;
    fa_auto* concatenation_result;
    fa_auto* complement_result;        // Complement of A (relative to alphabet)
    fa_auto* reverse_result;
    fa_auto* kleene_star_result;
    fa_auto* minimized_result;         // Minimized version of A
    fa_auto* determinized_result;      // Determinized version of A
    
    // Metadata about which operations were performed
    int performed_ops;
    char error_message[256];
    bool success;
} fa_operation_results;


#define FA_MINIMIZE_USES_ALGO(flags, algo) (((flags) & 0x0F) & (algo))

// Main composition function
fa_operation_results* fa_auto_compose(const fa_auto* a, const fa_auto* b, 
                                     fa_operation_flags flags);


fa_operation_results* fa_auto_stack_compose(fa_stack* stack, 
                                           fa_operation_flags flags, fa_composition_mode mode);

fa_operation_results* fa_auto_ncompose(fa_operation_flags flags,
                                       fa_composition_mode mode, 
                                       int count, ...);


fa_operation_results* fa_auto_vcompose(fa_operation_flags flags,
                                       fa_composition_mode mode, 
                                       fa_auto* first, ...);  
                                       
fa_stack* fa_stack_from_args(int count, ...);
fa_stack* fa_stack_from_null_terminated(fa_auto* first, ...);




// Individual operation functions (still available for direct use)

/**
 * @brief Computes the union of two automata (L1 ∪ L2).
 * @param a1 First automaton
 * @param a2 Second automaton
 * @return New automaton accepting L(a1) ∪ L(a2)
 */

fa_auto* fa_auto_union(const fa_auto* a1, const fa_auto* a2);


/**
 * @brief Computes the product (intersection) of two automata.
 * @param a1 First automaton
 * @param a2 Second automaton
 * @return New automaton accepting L(a1) ∩ L(a2)
 */
fa_auto* fa_auto_product(const fa_auto* a1, const fa_auto* a2);


/**
 * @brief Concatenates two automata (L1 · L2).
 * @param a1 First automaton
 * @param a2 Second automaton
 * @return New automaton accepting L(a1) · L(a2)
 */
fa_auto* fa_auto_concat(const fa_auto* a1, const fa_auto* a2);

fa_auto* fa_auto_difference(const fa_auto* a, const fa_auto* b);
fa_auto* fa_auto_symmetric_difference(const fa_auto* a, const fa_auto* b);
fa_auto* fa_auto_complement(const fa_auto* a);
fa_auto* fa_auto_reverse(const fa_auto* a);



//Unary Operations


/**
 * Apply Kleene closure operation to automaton
 * 
 * @param automaton The automaton to modify
 * @param flags FA_KLEENE_STAR for zero-or-more (a*)
 *             FA_KLEENE_PLUS for one-or-more (a+)
 * @return fa_auto
 */
fa_auto* fa_auto_kleene(fa_auto* automaton, fa_kleene_type type);
fa_auto* fa_auto_minimize(const fa_auto* a, fa_minimize_algorithm algorithm);
fa_auto* fa_auto_minimize_moore(const fa_auto *automaton);
fa_auto* fa_auto_minimize_hopcroft(const fa_auto *automaton);
fa_auto* fa_auto_minimize_table(const fa_auto *automaton);
fa_auto* fa_auto_minimize_brzozowski(const fa_auto *automaton);
fa_auto* fa_auto_optimize(fa_auto* automaton, fa_minimize_algorithm min_algo, fa_determinize_algorithm det_algo);
fa_auto* fa_auto_determinize(const fa_auto* a, fa_determinize_algorithm algorithm);

// Result management functions
void fa_operation_results_destroy(fa_operation_results* results);
void fa_operation_results_print(const fa_operation_results* results);

#endif // FA_OPERATIONS_H