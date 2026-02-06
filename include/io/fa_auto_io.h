#ifndef FA_AUTO_IO_H
#define FA_AUTO_IO_H

#include "../fa/fa.h"
#include "../fa_error.h"
#include "../fa_styles.h"
#include <stdio.h>


// Export functions

/**
 * @brief Export automaton to DOT format and write to a file.
 * 
 * @param automaton The automaton to export
 * @param filename The output filename
 * @param style The style to apply when generating the dot file
 * @return 0 on success, non-zero on error
 */
fa_error_t fa_auto_export_dot_file(const fa_auto* automaton, const char* filename, const fa_styles_dot_style_t* style);

/**
 * @brief Export automaton to DOT format and write to a stream.
 * 
 * @param automaton The automaton to export
 * @param stream The output stream
 * @param style The style to apply when generating the dot file
 * @return 0 on success, non-zero on error
 */
fa_error_t fa_auto_export_dot_stream(const fa_auto* automaton, FILE* stream, const fa_styles_dot_style_t* style);


/**
 * @brief Export automaton to JSON format and write to a file.
 * 
 * @param automaton The automaton to export
 * @param filename The output filename
 * @return 0 on success, non-zero on error
 */
fa_error_t fa_auto_export_json_file(const fa_auto* automaton, const char* filename);


/**
 * @brief Export automaton to JSON format and write to a stream.
 * 
 * @param automaton The automaton to export
 * @param stream The output stream
 * @return 0 on success, non-zero on error
 */
fa_error_t fa_auto_export_json_stream(const fa_auto* automaton, FILE* stream);

// Import functions
fa_auto* fa_auto_import_dot_file(const char* filename, int* error);
fa_auto* fa_auto_import_dot_stream(FILE* stream, int* error);
fa_auto* fa_auto_import_json_file(const char* filename, int* error);
fa_auto* fa_auto_import_json_stream(FILE* stream, int* error);



// Helper function to get error message
const char* fa_io_strerror(int error_code);

#endif // FA_AUTO_IO_H