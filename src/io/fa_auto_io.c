#include "../../include/io/fa_auto_io.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <jansson.h> 




static char* fa_auto_escape_dot_label(const char* label) {
    if (label == NULL) {
        return strdup("NULL");
    }
    
    // Calculate required buffer size
    size_t len = strlen(label);
    size_t escaped_len = 0;
    
    // First pass: count special characters
    for (size_t i = 0; i < len; i++) {
        if (label[i] == '\"' || label[i] == '\\') {
            escaped_len += 2;  // Need to escape it
        } else {
            escaped_len += 1;
        }
    }
    
    // Allocate buffer
    char* escaped = malloc(escaped_len + 1);
    if (escaped == NULL) {
        return NULL;
    }
    
    // Second pass: copy and escape
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        if (label[i] == '\"' || label[i] == '\\') {
            escaped[j++] = '\\';
        }
        escaped[j++] = label[i];
    }
    escaped[j] = '\0';
    
    return escaped;
}

fa_error_t fa_auto_export_dot_stream(const fa_auto* automaton, FILE* stream){
    
    if (automaton == NULL || stream == NULL) {
        return FA_ERR_NULL_ARGUMENT;
    }

    // Metadata
    fprintf(stream, "digraph Automaton {\n");
    
    fprintf(stream, "  // Automaton properties\n");
    fprintf(stream, "  rankdir=LR;                // Left-to-right layout\n");
    fprintf(stream, "  node [shape=circle];       // Default shape for states\n");

    
    fprintf(stream, "  // Metadata\n");
    fprintf(stream, "  label=\"States: %d, Alphabet size: %d\";\n",
            automaton->nstates, automaton->alphabet->length);
    fprintf(stream, "  labelloc=top;\n");
    fprintf(stream, "  fontsize=12;\n\n");

    printf("Finished Printing meta data");
    
    // Alphabet
    fprintf(stream, "  // Alphabet: ");
    set_fprint(automaton->alphabet, fprint_string, stream);
    fprintf(stream, "\n\n");
    
    printf("Finished Printing Alphabet");
    
    fprintf(stream, "  // State declarations\n");
    for (size_t i = 0; i < automaton->nstates; i++) {
        fa_state* state = automaton->states[i];
        
        
        const char* shape = "circle";
        const char* style = "";
        const char* fillcolor = "";
        int peripheries = 1;
        
        if (state->is_start && state->is_accept) {
            shape = "doublecircle";
            style = "filled";
            fillcolor = "lightyellow";
        } else if (state->is_start) {
            style = "filled";
            fillcolor = "lightyellow";
        } else if (state->is_accept) {
            peripheries = 2;
        }
        
        
        char* escaped_label = fa_auto_escape_dot_label(state->label);
        
        // States
        fprintf(stream, "  \"%s\" [", escaped_label);
        
        bool first_attr = true;
        if (strcmp(shape, "circle") != 0) {
            fprintf(stream, "shape=%s", shape);
            first_attr = false;
        }
        
        if (peripheries != 1) {
            if (!first_attr) fprintf(stream, ", ");
            fprintf(stream, "peripheries=%d", peripheries);
            first_attr = false;
        }
        
        if (style[0] != '\0') {
            if (!first_attr) fprintf(stream, ", ");
            fprintf(stream, "style=%s", style);
            first_attr = false;
        }
        
        if (fillcolor[0] != '\0') {
            if (!first_attr) fprintf(stream, ", ");
            fprintf(stream, "fillcolor=%s", fillcolor);
            first_attr = false;
        }
        
        
        if (state->ntrans > 0) {
            if (!first_attr) fprintf(stream, ", ");
            fprintf(stream, "tooltip=\"%d transitions\"", state->ntrans);
        }
        
        fprintf(stream, "];\n");
        
        free(escaped_label);
    }
    
    fprintf(stream, "\n");
    
    // Transitions
    fprintf(stream, "  // Transitions\n");
    for (size_t i = 0; i < automaton->nstates; i++) {
        fa_state* state = automaton->states[i];
        fa_trans* trans = state->trans;
        
        char* src_escaped = fa_auto_escape_dot_label(state->label);
        
        while (trans != NULL) {
            
            char* dest_escaped = fa_auto_escape_dot_label(trans->dest->label);
            
            
            char* symbol_escaped = fa_auto_escape_dot_label(trans->symbol);
            
            
            fprintf(stream, "  \"%s\" -> \"%s\" [label=\"%s\"];\n",
                    src_escaped, dest_escaped, symbol_escaped);
            
            free(dest_escaped);
            free(symbol_escaped);
            trans = trans->next;
        }
        
        free(src_escaped);
    }
    
    fprintf(stream, "}\n");
    
    return FA_SUCCESS;
}

fa_error_t fa_auto_export_dot_file(const fa_auto* automaton, const char* filename){
    if (automaton == NULL || filename == NULL) {
        return FA_ERR_NULL_ARGUMENT;  
    }
    
    FILE* dot_file = fopen(filename, "w");
    if (dot_file == NULL) {
        return FA_ERR_IO_FILE_OPEN;  
    }
    
    printf("Here!\n");
    fa_error_t result = fa_auto_export_dot_stream(automaton, dot_file);
    printf("Here 2!\n");
    fclose(dot_file);
    
    return result;
}


fa_error_t fa_auto_export_json_file(const fa_auto* automaton, const char* filename) {
    if (automaton == NULL || filename == NULL) {
        return FA_ERR_NULL_ARGUMENT;  
    }
    
    FILE* json_file = fopen(filename, "w");
    if (json_file == NULL) {
        return FA_ERR_IO_FILE_OPEN;  
    }
    
    int result = fa_auto_export_json_stream(automaton, json_file);
    fclose(json_file);
    
    return result;
}


static char* json_escape(const char* str) {
    if (str == NULL) return strdup("null");
        
    size_t len = strlen(str);
    size_t escaped_len = 0;
        
    // Count characters that need escaping
    for (size_t i = 0; i < len; i++) {
        switch (str[i]) {
            case '\"': case '\\': case '\b': case '\f': 
            case '\n': case '\r': case '\t':
                escaped_len += 2;  // \ + escaped char
                break;
            default:
                escaped_len += 1;
                break;
        }
    }
        
    char* escaped = malloc(escaped_len + 1);
    if (!escaped) return NULL;
        
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        switch (str[i]) {
            case '\"': escaped[j++] = '\\'; escaped[j++] = '\"'; break;
            case '\\': escaped[j++] = '\\'; escaped[j++] = '\\'; break;
            case '\b': escaped[j++] = '\\'; escaped[j++] = 'b'; break;
            case '\f': escaped[j++] = '\\'; escaped[j++] = 'f'; break;
            case '\n': escaped[j++] = '\\'; escaped[j++] = 'n'; break;
            case '\r': escaped[j++] = '\\'; escaped[j++] = 'r'; break;
            case '\t': escaped[j++] = '\\'; escaped[j++] = 't'; break;
            default:   escaped[j++] = str[i]; break;
        }
    }
    escaped[j] = '\0';
    return escaped;
}

fa_error_t fa_auto_export_json_stream(const fa_auto* automaton, FILE* stream) {
    if (automaton == NULL || stream == NULL) {
        return FA_ERR_NULL_ARGUMENT;
    }
    
    // Helper to write comma except for first item
    bool first_state = true;
    bool first_transition = true;
    
    //JSON object
    fprintf(stream, "{\n");
    
    // Metadata section
    fprintf(stream, "  \"metadata\": {\n");
    fprintf(stream, "    \"type\": \"finite_automaton\",\n");
    fprintf(stream, "    \"state_count\": %d,\n", automaton->nstates);
    fprintf(stream, "    \"alphabet_size\": %d,\n", automaton->alphabet->length);
    
    // Alphabet
    fprintf(stream, "    \"alphabet\": [ ");
    
    set_fprint(automaton->alphabet, fprint_string, stream);
    
    fprintf(stream, " ]\n");
    fprintf(stream, "  },\n");
    
    // States section
    fprintf(stream, "  \"states\": [\n");
    
    for (size_t i = 0; i < automaton->nstates; i++) {
        fa_state* state = automaton->states[i];
        
        if (!first_state) {
            fprintf(stream, ",\n");
        }
        first_state = false;
        
        char* label_escaped = json_escape(state->label);
        
        fprintf(stream, "    {\n");
        fprintf(stream, "      \"id\": %zu,\n", i);
        fprintf(stream, "      \"label\": \"%s\",\n", label_escaped);
        fprintf(stream, "      \"is_start\": %s,\n", state->is_start ? "true" : "false");
        fprintf(stream, "      \"is_accept\": %s,\n", state->is_accept ? "true" : "false");
        fprintf(stream, "      \"outgoing_transition_count\": %d,\n", state->ntrans);
        
        // Transitions for this state
        fprintf(stream, "      \"transitions\": [\n");
        
        fa_trans* trans = state->trans;
        first_transition = true;
        
        while (trans != NULL) {
            if (!first_transition) {
                fprintf(stream, ",\n");
            }
            first_transition = false;
            
            char* symbol_escaped = json_escape(trans->symbol);
            char* dest_label_escaped = json_escape(trans->dest->label);
            
            // Find destination state index
            int dest_index = -1;
            for (size_t j = 0; j < automaton->nstates; j++) {
                if (automaton->states[j] == trans->dest) {
                    dest_index = j;
                    break;
                }
            }
            
            fprintf(stream, "        {\n");
            fprintf(stream, "          \"symbol\": \"%s\",\n", symbol_escaped);
            fprintf(stream, "          \"destination\": {\n");
            fprintf(stream, "            \"id\": %d,\n", dest_index);
            fprintf(stream, "            \"label\": \"%s\"\n", dest_label_escaped);
            fprintf(stream, "          }\n");
            fprintf(stream, "        }");
            
            free(symbol_escaped);
            free(dest_label_escaped);
            trans = trans->next;
        }
        
        fprintf(stream, "\n      ]\n");
        fprintf(stream, "    }");
        
        free(label_escaped);
    }
    
    fprintf(stream, "\n  ],\n");
    
    // Statistics section
    fprintf(stream, "  \"statistics\": {\n");
    
    // Count start and accept states
    int start_count = 0;
    int accept_count = 0;
    int total_transitions = 0;
    
    for (size_t i = 0; i < automaton->nstates; i++) {
        fa_state* state = automaton->states[i];
        if (state->is_start) start_count++;
        if (state->is_accept) accept_count++;
        total_transitions += state->ntrans;
    }
    
    fprintf(stream, "    \"start_state_count\": %d,\n", start_count);
    fprintf(stream, "    \"accept_state_count\": %d,\n", accept_count);
    fprintf(stream, "    \"total_transitions\": %d,\n", total_transitions);
    
    // Calculate average transitions per state
    double avg_transitions = automaton->nstates > 0 ? 
                            (double)total_transitions / automaton->nstates : 0.0;
    fprintf(stream, "    \"average_transitions_per_state\": %.2f\n", avg_transitions);
    
    fprintf(stream, "  }\n");
    
    // End JSON object
    fprintf(stream, "}\n");
    
    return FA_SUCCESS;
}


// //Export files
// // Internal structure to track states during parsing
// typedef struct {
//     char* label;
//     bool is_start;
//     bool is_accept;
//     // Temporary storage for transitions
//     struct {
//         char* symbol;
//         char* dest_label;
//     } *transitions;
//     int trans_count;
//     int trans_capacity;
// } parse_state_t;

// // Helper to unescape DOT labels (simplified)
// static char* dot_unescape_label(const char* escaped) {
//     if (escaped == NULL) return NULL;
    
//     size_t len = strlen(escaped);
//     char* unescaped = malloc(len + 1);
//     if (!unescaped) return NULL;
    
//     size_t j = 0;
//     for (size_t i = 0; i < len; i++) {
//         if (escaped[i] == '\\' && i + 1 < len) {
//             i++; // Skip the backslash
//         }
//         unescaped[j++] = escaped[i];
//     }
//     unescaped[j] = '\0';
    
//     // Remove quotes if present
//     if (j > 1 && unescaped[0] == '"' && unescaped[j-1] == '"') {
//         unescaped[j-1] = '\0';
//         memmove(unescaped, unescaped + 1, j - 1);
//     }
    
//     return unescaped;
// }

// // Parse a DOT node declaration
// static int parse_dot_node_declaration(const char* line, parse_state_t** states, 
//                                       int* state_count, int* state_capacity) {
//     // Find label (before '[')
//     const char* bracket = strchr(line, '[');
//     if (!bracket) return FA_IO_ERROR_INVALID_FORMAT;
    
//     // Extract label
//     const char* label_start = line;
//     const char* label_end = bracket;
    
//     // Skip whitespace
//     while (label_start < label_end && isspace(*label_start)) label_start++;
//     while (label_end > label_start && isspace(*(label_end-1))) label_end--;
    
//     char* label = malloc(label_end - label_start + 1);
//     if (!label) return FA_IO_ERROR_MEMORY;
//     strncpy(label, label_start, label_end - label_start);
//     label[label_end - label_start] = '\0';
    
//     char* unescaped_label = dot_unescape_label(label);
//     free(label);
//     if (!unescaped_label) return FA_IO_ERROR_MEMORY;
    
//     // Check if state already exists
//     int state_index = -1;
//     for (int i = 0; i < *state_count; i++) {
//         if (strcmp(states[i]->label, unescaped_label) == 0) {
//             state_index = i;
//             free(unescaped_label);
//             break;
//         }
//     }
    
//     if (state_index == -1) {
//         // Add new state
//         if (*state_count >= *state_capacity) {
//             *state_capacity *= 2;
//             parse_state_t** new_states = realloc(states, 
//                                                  sizeof(parse_state_t*) * (*state_capacity));
//             if (!new_states) {
//                 free(unescaped_label);
//                 return FA_IO_ERROR_MEMORY;
//             }
//             *states = new_states;
//         }
        
//         parse_state_t* new_state = calloc(1, sizeof(parse_state_t));
//         if (!new_state) {
//             free(unescaped_label);
//             return FA_IO_ERROR_MEMORY;
//         }
        
//         new_state->label = unescaped_label;
//         new_state->is_start = false;
//         new_state->is_accept = false;
//         new_state->transitions = NULL;
//         new_state->trans_count = 0;
//         new_state->trans_capacity = 0;
        
//         state_index = *state_count;
//         (*states)[state_index] = new_state;
//         (*state_count)++;
//     } else {
//         unescaped_label = states[state_index]->label;
//     }
    
//     // Parse attributes inside [ ]
//     const char* attr_start = bracket + 1;
//     const char* attr_end = strchr(attr_start, ']');
//     if (!attr_end) return FA_IO_ERROR_INVALID_FORMAT;
    
//     char* attrs = malloc(attr_end - attr_start + 1);
//     if (!attrs) return FA_IO_ERROR_MEMORY;
//     strncpy(attrs, attr_start, attr_end - attr_start);
//     attrs[attr_end - attr_start] = '\0';
    
//     // Parse individual attributes
//     char* saveptr;
//     char* token = strtok_r(attrs, ",", &saveptr);
//     while (token) {
//         // Trim whitespace
//         char* start = token;
//         char* end = start + strlen(start) - 1;
//         while (isspace(*start)) start++;
//         while (end > start && isspace(*end)) end--;
//         end[1] = '\0';
        
//         // Check for key=value pairs
//         char* equals = strchr(start, '=');
//         if (equals) {
//             *equals = '\0';
//             char* key = start;
//             char* value = equals + 1;
            
//             // Trim value
//             while (isspace(*value)) value++;
//             char* value_end = value + strlen(value) - 1;
//             while (value_end > value && isspace(*value_end)) value_end--;
//             value_end[1] = '\0';
            
//             // Remove quotes from value if present
//             if (value[0] == '"' && value[strlen(value)-1] == '"') {
//                 value[strlen(value)-1] = '\0';
//                 memmove(value, value + 1, strlen(value));
//             }
            
//             if (strcmp(key, "shape") == 0) {
//                 if (strstr(value, "doublecircle")) {
//                     states[state_index]->is_accept = true;
//                 }
//             } else if (strcmp(key, "style") == 0) {
//                 if (strstr(value, "filled")) {
//                     states[state_index]->is_start = true;
//                 }
//             } else if (strcmp(key, "peripheries") == 0) {
//                 if (atoi(value) == 2) {
//                     states[state_index]->is_accept = true;
//                 }
//             } else if (strcmp(key, "fillcolor") == 0) {
//                 if (strstr(value, "lightyellow")) {
//                     states[state_index]->is_start = true;
//                 }
//             }
//         }
        
//         token = strtok_r(NULL, ",", &saveptr);
//     }
    
//     free(attrs);
//     return FA_IO_SUCCESS;
// }

// // Parse a DOT edge declaration
// static int parse_dot_edge_declaration(const char* line, parse_state_t** states, 
//                                       int state_count) {
//     // Format: source -> dest [label="symbol"]
//     const char* arrow = strstr(line, "->");
//     if (!arrow) return FA_IO_ERROR_INVALID_FORMAT;
    
//     // Extract source label
//     const char* source_start = line;
//     const char* source_end = arrow;
    
//     // Skip whitespace
//     while (source_start < source_end && isspace(*source_start)) source_start++;
//     while (source_end > source_start && isspace(*(source_end-1))) source_end--;
    
//     char* source_label = malloc(source_end - source_start + 1);
//     if (!source_label) return FA_IO_ERROR_MEMORY;
//     strncpy(source_label, source_start, source_end - source_start);
//     source_label[source_end - source_start] = '\0';
    
//     char* unescaped_source = dot_unescape_label(source_label);
//     free(source_label);
//     if (!unescaped_source) return FA_IO_ERROR_MEMORY;
    
//     // Find destination (before '[' or end of line)
//     const char* dest_start = arrow + 2; // Skip "->"
//     const char* bracket = strchr(dest_start, '[');
//     const char* dest_end = bracket ? bracket : dest_start + strlen(dest_start);
    
//     // Trim destination
//     while (dest_start < dest_end && isspace(*dest_start)) dest_start++;
//     while (dest_end > dest_start && isspace(*(dest_end-1))) dest_end--;
    
//     char* dest_label = malloc(dest_end - dest_start + 1);
//     if (!dest_label) {
//         free(unescaped_source);
//         return FA_IO_ERROR_MEMORY;
//     }
//     strncpy(dest_label, dest_start, dest_end - dest_start);
//     dest_label[dest_end - dest_start] = '\0';
    
//     char* unescaped_dest = dot_unescape_label(dest_label);
//     free(dest_label);
//     if (!unescaped_dest) {
//         free(unescaped_source);
//         return FA_IO_ERROR_MEMORY;
//     }
    
//     // Find source and destination states
//     int source_index = -1, dest_index = -1;
//     for (int i = 0; i < state_count; i++) {
//         if (strcmp(states[i]->label, unescaped_source) == 0) {
//             source_index = i;
//         }
//         if (strcmp(states[i]->label, unescaped_dest) == 0) {
//             dest_index = i;
//         }
//     }
    
//     if (source_index == -1 || dest_index == -1) {
//         free(unescaped_source);
//         free(unescaped_dest);
//         return FA_IO_ERROR_INVALID_STATE;
//     }
    
//     // Parse label attribute if present
//     char* symbol = strdup("ε"); // Default epsilon transition
//     if (bracket) {
//         const char* label_key = strstr(bracket, "label=");
//         if (label_key) {
//             label_key += 6; // Skip "label="
//             if (*label_key == '"') {
//                 label_key++; // Skip opening quote
//                 const char* label_end = strchr(label_key, '"');
//                 if (label_end) {
//                     free(symbol);
//                     symbol = malloc(label_end - label_key + 1);
//                     if (symbol) {
//                         strncpy(symbol, label_key, label_end - label_key);
//                         symbol[label_end - label_key] = '\0';
//                     }
//                 }
//             }
//         }
//     }
    
//     if (!symbol) {
//         free(unescaped_source);
//         free(unescaped_dest);
//         return FA_IO_ERROR_MEMORY;
//     }
    
//     // Add transition to source state
//     parse_state_t* state = states[source_index];
//     if (state->trans_count >= state->trans_capacity) {
//         state->trans_capacity = state->trans_capacity ? state->trans_capacity * 2 : 4;
//         void* new_trans = realloc(state->transitions, 
//                                  sizeof(state->transitions[0]) * state->trans_capacity);
//         if (!new_trans) {
//             free(symbol);
//             free(unescaped_source);
//             free(unescaped_dest);
//             return FA_IO_ERROR_MEMORY;
//         }
//         state->transitions = new_trans;
//     }
    
//     state->transitions[state->trans_count].symbol = symbol;
//     state->transitions[state->trans_count].dest_label = strdup(unescaped_dest);
//     state->trans_count++;
    
//     free(unescaped_source);
//     free(unescaped_dest);
//     return FA_IO_SUCCESS;
// }

// fa_auto* fa_auto_import_dot_stream(FILE* stream, int* error) {
//     if (stream == NULL) {
//         if (error) *error = FA_IO_ERROR_NULL_ARGUMENT;
//         return NULL;
//     }
    
//     parse_state_t** states = NULL;
//     int state_count = 0;
//     int state_capacity = 16;
    
//     states = malloc(sizeof(parse_state_t*) * state_capacity);
//     if (!states) {
//         if (error) *error = FA_IO_ERROR_MEMORY;
//         return NULL;
//     }
    
//     char line[1024];
//     bool in_automaton = false;
//     int line_num = 0;
    
//     while (fgets(line, sizeof(line), stream)) {
//         line_num++;
        
//         // Remove trailing newline
//         line[strcspn(line, "\n")] = '\0';
        
//         // Skip empty lines and comments
//         char* trimmed = line;
//         while (isspace(*trimmed)) trimmed++;
//         if (*trimmed == '\0' || *trimmed == '#') continue;
        
//         // Check for digraph declaration
//         if (strstr(trimmed, "digraph") || strstr(trimmed, "graph")) {
//             in_automaton = true;
//             continue;
//         }
        
//         if (!in_automaton) continue;
        
//         // Check for end of graph
//         if (strcmp(trimmed, "}") == 0) {
//             break;
//         }
        
//         // Skip metadata lines (starting with // or containing = without ->)
//         if (strstr(trimmed, "//") == trimmed || 
//             (strchr(trimmed, '=') && !strstr(trimmed, "->"))) {
//             continue;
//         }
        
//         // Parse node declaration (contains [ but not ->)
//         if (strchr(trimmed, '[') && !strstr(trimmed, "->")) {
//             int result = parse_dot_node_declaration(trimmed, states, 
//                                                    &state_count, &state_capacity);
//             if (result != FA_IO_SUCCESS) {
//                 if (error) *error = result;
//                 // Cleanup
//                 for (int i = 0; i < state_count; i++) {
//                     free(states[i]->label);
//                     for (int j = 0; j < states[i]->trans_count; j++) {
//                         free(states[i]->transitions[j].symbol);
//                         free(states[i]->transitions[j].dest_label);
//                     }
//                     free(states[i]->transitions);
//                     free(states[i]);
//                 }
//                 free(states);
//                 return NULL;
//             }
//         }
//         // Parse edge declaration (contains ->)
//         else if (strstr(trimmed, "->")) {
//             int result = parse_dot_edge_declaration(trimmed, states, state_count);
//             if (result != FA_IO_SUCCESS) {
//                 if (error) *error = result;
//                 // Cleanup
//                 for (int i = 0; i < state_count; i++) {
//                     free(states[i]->label);
//                     for (int j = 0; j < states[i]->trans_count; j++) {
//                         free(states[i]->transitions[j].symbol);
//                         free(states[i]->transitions[j].dest_label);
//                     }
//                     free(states[i]->transitions);
//                     free(states[i]);
//                 }
//                 free(states);
//                 return NULL;
//             }
//         }
//     }
    
//     // Create automaton from parsed data
//     fa_auto* automaton = malloc(sizeof(fa_auto));
//     if (!automaton) {
//         if (error) *error = FA_IO_ERROR_MEMORY;
//         goto cleanup;
//     }
    
//     automaton->nstates = state_count;
//     automaton->states = malloc(sizeof(fa_state*) * state_count);
//     if (!automaton->states) {
//         if (error) *error = FA_IO_ERROR_MEMORY;
//         free(automaton);
//         goto cleanup;
//     }
    
//     // Create alphabet set (you'll need to initialize your Set type)
//     automaton->alphabet = set_create(); // Implement this based on your Set API
    
//     // Create states
//     for (int i = 0; i < state_count; i++) {
//         fa_state* state = malloc(sizeof(fa_state));
//         if (!state) {
//             if (error) *error = FA_IO_ERROR_MEMORY;
//             goto cleanup_automaton;
//         }
        
//         state->label = strdup(states[i]->label);
//         state->is_start = states[i]->is_start;
//         state->is_accept = states[i]->is_accept;
//         state->trans = NULL;
//         state->ntrans = states[i]->trans_count;
        
//         automaton->states[i] = state;
        
//         // Add symbols to alphabet
//         for (int j = 0; j < states[i]->trans_count; j++) {
//             set_add(automaton->alphabet, states[i]->transitions[j].symbol);
//         }
//     }
    
//     // Create transitions
//     for (int i = 0; i < state_count; i++) {
//         fa_state* src_state = automaton->states[i];
//         fa_trans** last_trans = &src_state->trans;
        
//         for (int j = 0; j < states[i]->trans_count; j++) {
//             // Find destination state
//             fa_state* dest_state = NULL;
//             for (int k = 0; k < state_count; k++) {
//                 if (strcmp(automaton->states[k]->label, 
//                           states[i]->transitions[j].dest_label) == 0) {
//                     dest_state = automaton->states[k];
//                     break;
//                 }
//             }
            
//             if (!dest_state) continue;
            
//             fa_trans* trans = malloc(sizeof(fa_trans));
//             if (!trans) {
//                 if (error) *error = FA_IO_ERROR_MEMORY;
//                 goto cleanup_automaton;
//             }
            
//             trans->symbol = strdup(states[i]->transitions[j].symbol);
//             trans->dest = dest_state;
//             trans->src = src_state;
//             trans->next = NULL;
            
//             *last_trans = trans;
//             last_trans = &trans->next;
//         }
//     }
    
//     if (error) *error = FA_IO_SUCCESS;
    
// cleanup:
//     // Free parse structures
//     for (int i = 0; i < state_count; i++) {
//         free(states[i]->label);
//         for (int j = 0; j < states[i]->trans_count; j++) {
//             free(states[i]->transitions[j].symbol);
//             free(states[i]->transitions[j].dest_label);
//         }
//         free(states[i]->transitions);
//         free(states[i]);
//     }
//     free(states);
    
//     return automaton;
    
// cleanup_automaton:
//     // Cleanup partially created automaton
//     if (automaton) {
//         if (automaton->states) {
//             for (int i = 0; i < state_count; i++) {
//                 if (automaton->states[i]) {
//                     free(automaton->states[i]->label);
//                     fa_trans* trans = automaton->states[i]->trans;
//                     while (trans) {
//                         fa_trans* next = trans->next;
//                         free(trans->symbol);
//                         free(trans);
//                         trans = next;
//                     }
//                     free(automaton->states[i]);
//                 }
//             }
//             free(automaton->states);
//         }
//         if (automaton->alphabet) set_destroy(automaton->alphabet);
//         free(automaton);
//     }
//     goto cleanup;
// }

// fa_auto* fa_auto_import_dot_file(const char* filename, int* error) {
//     if (filename == NULL) {
//         if (error) *error = FA_IO_ERROR_NULL_ARGUMENT;
//         return NULL;
//     }
    
//     FILE* file = fopen(filename, "r");
//     if (!file) {
//         if (error) *error = FA_IO_ERROR_FILE_OPEN;
//         return NULL;
//     }
    
//     fa_auto* automaton = fa_auto_import_dot_stream(file, error);
//     fclose(file);
    
//     return automaton;
// }


// fa_auto* fa_auto_import_json_stream(FILE* stream, int* error) {
//     if (stream == NULL) {
//         if (error) *error = FA_IO_ERROR_NULL_ARGUMENT;
//         return NULL;
//     }
    
//     // Read entire file
//     fseek(stream, 0, SEEK_END);
//     long file_size = ftell(stream);
//     fseek(stream, 0, SEEK_SET);
    
//     char* json_str = malloc(file_size + 1);
//     if (!json_str) {
//         if (error) *error = FA_IO_ERROR_MEMORY;
//         return NULL;
//     }
    
//     fread(json_str, 1, file_size, stream);
//     json_str[file_size] = '\0';
    
//     // Parse JSON (using jansson library as example)
//     json_error_t json_error;
//     json_t* root = json_loads(json_str, 0, &json_error);
//     free(json_str);
    
//     if (!root) {
//         if (error) *error = FA_IO_ERROR_PARSE_FAILED;
//         return NULL;
//     }
    
//     // Extract metadata
//     json_t* metadata = json_object_get(root, "metadata");
//     if (!metadata) {
//         json_decref(root);
//         if (error) *error = FA_IO_ERROR_INVALID_FORMAT;
//         return NULL;
//     }
    
//     // Create automaton
//     fa_auto* automaton = malloc(sizeof(fa_auto));
//     if (!automaton) {
//         json_decref(root);
//         if (error) *error = FA_IO_ERROR_MEMORY;
//         return NULL;
//     }
    
//     automaton->nstates = json_integer_value(json_object_get(metadata, "state_count"));
//     automaton->alphabet = set_create();
//     automaton->states = malloc(sizeof(fa_state*) * automaton->nstates);
    
//     if (!automaton->states) {
//         json_decref(root);
//         free(automaton);
//         if (error) *error = FA_IO_ERROR_MEMORY;
//         return NULL;
//     }
    
//     // Parse alphabet
//     json_t* alphabet = json_object_get(metadata, "alphabet");
//     if (json_is_array(alphabet)) {
//         size_t index;
//         json_t* value;
//         json_array_foreach(alphabet, index, value) {
//             if (json_is_string(value)) {
//                 set_add(automaton->alphabet, json_string_value(value));
//             }
//         }
//     }
    
//     // Parse states
//     json_t* states = json_object_get(root, "states");
//     if (!json_is_array(states) || json_array_size(states) != automaton->nstates) {
//         json_decref(root);
//         set_destroy(automaton->alphabet);
//         free(automaton->states);
//         free(automaton);
//         if (error) *error = FA_IO_ERROR_INVALID_FORMAT;
//         return NULL;
//     }
    
//     // First pass: create all states
//     for (size_t i = 0; i < automaton->nstates; i++) {
//         json_t* state_obj = json_array_get(states, i);
        
//         fa_state* state = malloc(sizeof(fa_state));
//         if (!state) {
//             // Cleanup partial allocation
//             for (size_t j = 0; j < i; j++) {
//                 free(automaton->states[j]->label);
//                 free(automaton->states[j]);
//             }
//             json_decref(root);
//             set_destroy(automaton->alphabet);
//             free(automaton->states);
//             free(automaton);
//             if (error) *error = FA_IO_ERROR_MEMORY;
//             return NULL;
//         }
        
//         state->label = strdup(json_string_value(json_object_get(state_obj, "label")));
//         state->is_start = json_is_true(json_object_get(state_obj, "is_start"));
//         state->is_accept = json_is_true(json_object_get(state_obj, "is_accept"));
//         state->trans = NULL;
//         state->ntrans = json_integer_value(json_object_get(state_obj, "transition_count"));
        
//         automaton->states[i] = state;
//     }
    
//     // Second pass: create transitions
//     for (size_t i = 0; i < automaton->nstates; i++) {
//         json_t* state_obj = json_array_get(states, i);
//         json_t* transitions = json_object_get(state_obj, "transitions");
        
//         fa_state* src_state = automaton->states[i];
//         fa_trans** last_trans = &src_state->trans;
        
//         size_t trans_index;
//         json_t* trans_obj;
//         json_array_foreach(transitions, trans_index, trans_obj) {
//             const char* symbol = json_string_value(json_object_get(trans_obj, "symbol"));
//             json_t* dest_obj = json_object_get(trans_obj, "destination");
//             int dest_id = json_integer_value(json_object_get(dest_obj, "id"));
            
//             if (dest_id < 0 || dest_id >= automaton->nstates) {
//                 // Invalid destination, skip
//                 continue;
//             }
            
//             fa_trans* trans = malloc(sizeof(fa_trans));
//             if (!trans) {
//                 // Cleanup
//                 json_decref(root);
//                 // TODO: Proper cleanup of partially built automaton
//                 return NULL;
//             }
            
//             trans->symbol = strdup(symbol);
//             trans->dest = automaton->states[dest_id];
//             trans->src = src_state;
//             trans->next = NULL;
            
//             *last_trans = trans;
//             last_trans = &trans->next;
            
//             // Add symbol to alphabet if not already present
//             set_add(automaton->alphabet, symbol);
//         }
//     }
    
//     json_decref(root);
    
//     if (error) *error = FA_IO_SUCCESS;
//     return automaton;
// }

// fa_auto* fa_auto_import_json_file(const char* filename, int* error) {
//     if (filename == NULL) {
//         if (error) *error = FA_IO_ERROR_NULL_ARGUMENT;
//         return NULL;
//     }
    
//     FILE* file = fopen(filename, "r");
//     if (!file) {
//         if (error) *error = FA_IO_ERROR_FILE_OPEN;
//         return NULL;
//     }
    
//     fa_auto* automaton = fa_auto_import_json_stream(file, error);
//     fclose(file);
    
//     return automaton;
// }


