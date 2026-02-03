#include "../include/fa_error.h"
#include <stdio.h>




const char* fa_error_str(fa_error_t err) {
    switch (err) {
        /* ===== SUCCESS ===== */
        case FA_SUCCESS:
            return "Success";
            
        /* ===== GENERIC ERRORS ===== */
        case FA_ERR_NULL_ARGUMENT:
            return "Null argument";
        case FA_ERR_OUT_OF_MEMORY:
            return "Out of memory";
        case FA_ERR_INVALID_ARGUMENT:
            return "Invalid argument";
        case FA_ERR_INTERNAL:
            return "Internal error";
        case FA_ERR_NOT_IMPLEMENTED:
            return "Not implemented";
        case FA_ERR_BUFFER_TOO_SMALL:
            return "Buffer too small";
        case FA_ERR_INVALID_OPERATION:
            return "Invalid operation";
            
        /* ===== SET & COLLECTION ERRORS ===== */
        case FA_ERR_SET_DUPLICATE:
            return "Duplicate element in set";
        case FA_ERR_SET_EMPTY:
            return "Set is empty";
        case FA_ERR_SET_NOT_FOUND:
            return "Element not found in set";
        case FA_ERR_SET_FULL:
            return "Set is full";
            
        /* ===== FINITE AUTOMATA ERRORS ===== */
        case FA_ERR_FA_INVALID_STATE:
            return "Invalid finite automaton state";
        case FA_ERR_FA_INVALID_SYMBOL:
            return "Invalid finite automaton symbol";
        case FA_ERR_FA_DUPLICATE_STATE:
            return "Duplicate finite automaton state";
        case FA_ERR_FA_DUPLICATE_TRANSITION:
            return "Duplicate finite automaton transition";
        case FA_ERR_FA_STATE_NOT_FOUND:
            return "State not found in automaton";
        case FA_ERR_FA_SYMBOL_NOT_IN_ALPHABET:
            return "Symbol not in alphabet";
        case FA_ERR_FA_NO_INITIAL_STATE:
            return "No initial state specified";
        case FA_ERR_FA_NO_FINAL_STATE:
            return "No final state specified";
        case FA_ERR_FA_MULTIPLE_INITIAL_STATES:
            return "Multiple initial states not allowed";
        case FA_ERR_FA_EPSILON_NOT_ALLOWED:
            return "Epsilon transitions not allowed";
        case FA_ERR_FA_EMPTY_ALPHABET:
            return "Alphabet is empty";
        case FA_ERR_FA_INVALID_TRANSITION:
            return "Invalid transition";
            
        /* ===== HASH TABLE ERRORS ===== */
        case FA_ERR_HASH_KEY_NOT_FOUND:
            return "Hash key not found";
        case FA_ERR_HASH_DUPLICATE_KEY:
            return "Duplicate hash key";
        case FA_ERR_HASH_TABLE_FULL:
            return "Hash table is full";
        case FA_ERR_HASH_INVALID_KEY:
            return "Invalid hash key";
            
        /* ===== I/O ERRORS ===== */
        case FA_ERR_IO_FILE_OPEN:
            return "Failed to open file";
        case FA_ERR_IO_FILE_NOT_FOUND:
            return "File not found";
        case FA_ERR_IO_PERMISSION_DENIED:
            return "Permission denied";
        case FA_ERR_IO_INVALID_FORMAT:
            return "Invalid file format";
        case FA_ERR_IO_PARSE_FAILED:
            return "Failed to parse input";
        case FA_ERR_IO_WRITE_FAILED:
            return "Failed to write output";
        case FA_ERR_IO_READ_FAILED:
            return "Failed to read input";
        case FA_ERR_IO_EOF:
            return "End of file reached";
        case FA_ERR_IO_MALFORMED_INPUT:
            return "Malformed input";
            
        /* ===== ITERATOR ERRORS ===== */
        case FA_ERR_ITERATOR_END:
            return "Iterator reached end";
        case FA_ERR_ITERATOR_INVALID:
            return "Invalid iterator";
        case FA_ERR_ITERATOR_NO_MORE_ELEMENTS:
            return "No more elements in iterator";
            
        /* ===== PUSHDOWN AUTOMATA ERRORS ===== */
        case FA_ERR_PDA_STACK_UNDERFLOW:
            return "Pushdown automaton stack underflow";
        case FA_ERR_PDA_STACK_OVERFLOW:
            return "Pushdown automaton stack overflow";
        case FA_ERR_PDA_INVALID_TRANSITION:
            return "Invalid pushdown automaton transition";
        case FA_ERR_PDA_INVALID_STACK_SYMBOL:
            return "Invalid stack symbol";
        case FA_ERR_PDA_EMPTY_STACK:
            return "Stack is empty";
            
        /* ===== TURING MACHINE ERRORS ===== */
        case FA_ERR_TM_INVALID_TAPE:
            return "Invalid Turing machine tape";
        case FA_ERR_TM_INVALID_TRANSITION:
            return "Invalid Turing machine transition";
        case FA_ERR_TM_TAPE_OUT_OF_BOUNDS:
            return "Tape position out of bounds";
        case FA_ERR_TM_INVALID_TAPE_SYMBOL:
            return "Invalid tape symbol";
        case FA_ERR_TM_NO_ACCEPTING_STATE:
            return "No accepting state defined";
        case FA_ERR_TM_NO_REJECTING_STATE:
            return "No rejecting state defined";
            
        /* ===== REGULAR EXPRESSION ERRORS ===== */
        case FA_ERR_REGEX_INVALID_PATTERN:
            return "Invalid regular expression pattern";
        case FA_ERR_REGEX_INVALID_ESCAPE:
            return "Invalid escape sequence";
        case FA_ERR_REGEX_UNBALANCED_PARENTHESES:
            return "Unbalanced parentheses";
        case FA_ERR_REGEX_UNEXPECTED_TOKEN:
            return "Unexpected token";
        case FA_ERR_REGEX_TRAILING_BACKSLASH:
            return "Trailing backslash";
            
        /* ===== CONVERSION ERRORS ===== */
        case FA_ERR_CONVERSION_FAILED:
            return "Conversion failed";
        case FA_ERR_CONVERSION_NOT_SUPPORTED:
            return "Conversion not supported";
            
        /* ===== VALIDATION ERRORS ===== */
        case FA_ERR_VALIDATION_FAILED:
            return "Validation failed";
        case FA_ERR_NOT_DETERMINISTIC:
            return "Automaton is not deterministic";
        case FA_ERR_NOT_COMPLETE:
            return "Automaton is not complete";
            
        /* ===== CONFIGURATION ERRORS ===== */
        case FA_ERR_CONFIG_INVALID:
            return "Invalid configuration";
        case FA_ERR_CONFIG_MISSING:
            return "Missing configuration";
            
        /* ===== NETWORK/REMOTE ERRORS ===== */
        case FA_ERR_NETWORK_FAILED:
            return "Network operation failed";
        case FA_ERR_NETWORK_TIMEOUT:
            return "Network timeout";
        case FA_ERR_NETWORK_CONNECTION_REFUSED:
            return "Connection refused";
            
        default:
            /* For unknown errors, return a generic message */
            static char unknown_buf[32];
            snprintf(unknown_buf, sizeof(unknown_buf), "Unknown error (%d)", err);
            return unknown_buf;
    }
}