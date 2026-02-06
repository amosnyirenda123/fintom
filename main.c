#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "include/fa/fa.h"             // Core finite automaton structures & functions
#include "include/fa/fa_operations.h"  // Operations on finite automata
#include "include/set/set.h"           // Generic set implementation (used for alphabet)
#include "include/io/fa_auto_io.h"     // Printing / I/O helpers for automata
#include "include/fa_error.h"          // Error codes returned by FA functions
#include "include/fa_styles.h"         // Styles to apply when generating dot files
#include "include/common.h"
#include "include/fa/fa_debug.h"



// Automaton for (a|b)*ab
fa_auto* create_automaton1() {
   
   const int states = 3;
   fa_auto* fa = fa_auto_create(states);
    
   // Create alphabet
   const char* symbols[] = {"a", "b"};
   const int nsymbols = sizeof(symbols) / sizeof(symbols[0]);
   Set* alphabet = set_create_string_set();
   size_t count = fa_alphabet_insert_symbols(alphabet, symbols, nsymbols);
    
   // Create states
   fa_state* q0 = fa_state_create("q0", true, false);
   fa_state* q1 = fa_state_create("q1", false, false);
   fa_state* q2 = fa_state_create("q2", false, true);
    
   // Create transitions
   fa_trans_create(q0, q1, "a");
   fa_trans_create(q0, q0, "b");
   fa_trans_create(q1, q1, "a");
   fa_trans_create(q1, q2, "b");
   fa_trans_create(q2, q1, "a");
   fa_trans_create(q2, q0, "b");
    
   // Assign to automaton
   fa->states[0] = q0;
   fa->states[1] = q1;
   fa->states[2] = q2;
   fa->nstates = states;
   fa->alphabet = alphabet;
    
   return fa;
}


// Automaton for even number of 'a's
fa_auto* create_automaton2() {

   // Alphabet
   const char* symbols[] = {"a", "b"};
   const int nsymbols = sizeof(symbols) / sizeof(symbols[0]);
   Set* alphabet = set_create_string_set();
   size_t count = fa_alphabet_insert_symbols(alphabet, symbols, nsymbols);


   const int states = 2;
   fa_auto* fa = fa_auto_create(states);

   // Create states
   fa_state* q0 = fa_state_create("q0", true, true);  // start and accepting
   fa_state* q1 = fa_state_create("q1", false, false);
    
   // Create transitions
   fa_trans_create(q0, q1, "a");
   fa_trans_create(q0, q0, "b");
   fa_trans_create(q1, q0, "a");
   fa_trans_create(q1, q1, "b");
    
   // Assign to automaton
   fa->states[0] = q0;
   fa->states[1] = q1;
   fa->nstates = states;
   fa->alphabet = alphabet;
    
   return fa;
}


void test_binary_operations() {
   fa_auto* A = create_automaton1();
   fa_auto* B = create_automaton2();

   fa_auto_export_dot_file(A, "../examples/binary_operations/a.dot", &FA_STYLES_DOT_STYLE_CLASSIC);
   fa_auto_export_dot_file(B, "../examples/binary_operations/b.dot", &FA_STYLES_DOT_STYLE_CLASSIC);
   
    
   // Test union
   fa_auto* union_fa = fa_auto_union(A, B);
   fa_auto_export_dot_file(union_fa, "../examples/binary_operations/a_union_b.dot", &FA_STYLES_DOT_STYLE_FANCY);
   fa_auto_export_json_file(union_fa, "../examples/binary_operations/a_union_b.json");
    
   printf("Testing Union (A U B):\n");
   printf("  'ab' should be accepted: %s\n", 
         fa_auto_accepts(union_fa, "ab") ? "PASS" : "FAIL");
   printf("  'aa' should be accepted: %s\n", 
         fa_auto_accepts(union_fa, "aa") ? "PASS" : "FAIL");
   printf("  'a' should be rejected: %s\n", 
         !fa_auto_accepts(union_fa, "a") ? "PASS" : "FAIL");
   printf("  '' should be accepted: %s\n", 
         fa_auto_accepts(union_fa, "") ? "PASS" : "FAIL");
    
   // Test concatenation
   fa_auto* concat_fa = fa_auto_concat(A, B);
   fa_auto_export_dot_file(concat_fa, "../examples/binary_operations/a_concat_b.dot", &FA_STYLES_DOT_STYLE_FANCY);
   fa_auto_export_json_file(concat_fa, "../examples/binary_operations/a_concat_b.json");

   printf("\nTesting Concatenation (A . B):\n");
   printf("  'ab' should be accepted: %s\n", 
         fa_auto_accepts(concat_fa, "ab") ? "PASS" : "FAIL");
   printf("  'abb' should be accepted: %s\n", 
         fa_auto_accepts(concat_fa, "abb") ? "PASS" : "FAIL");
   printf("  'aba' should be rejected: %s\n", 
         !fa_auto_accepts(concat_fa, "aba") ? "PASS" : "FAIL");
   printf("  'a' should be rejected: %s\n", 
         !fa_auto_accepts(concat_fa, "a") ? "PASS" : "FAIL");
    
    // Cleanup
   fa_auto_destroy(A);
   fa_auto_destroy(B);
   fa_auto_destroy(union_fa);
   fa_auto_destroy(concat_fa);
}



int main()
{
   test_binary_operations();
}
