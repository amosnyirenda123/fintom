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
   
   fa_auto* auto_from_symbol = fa_auto_from_symbol("a");


   fa_auto_print_verbose(auto_from_symbol);

   fa_auto_export_dot_file(auto_from_symbol, "../examples/symbol/from_symbol.dot");

   fa_auto_export_json_file(auto_from_symbol, "../examples/symbol/from_symbol.json");

   fa_auto_destroy(auto_from_symbol);

   
    
}
