#ifndef FA_STYLES_H
#define FA_STYLES_H

#include <stdbool.h>

typedef struct {
    // Layout options
    enum {
        FA_LAYOUT_LR,      // Left to right
        FA_LAYOUT_RL,      // Right to left  
        FA_LAYOUT_TB,      // Top to bottom
        FA_LAYOUT_BT       // Bottom to top
    } rankdir;
    
    // State styling
    enum {
        FA_STYLE_FILL,      // Fill colors for start/accept states
        FA_STYLE_PERIPHERY, // Double circles for accept states
        FA_STYLE_ARROWS,    // Use arrows for start/accept
        FA_STYLE_MINIMAL    // Simple circles only
    } style;
    
    // Node shape
    char* default_shape;    // "circle", "ellipse", "box", "diamond", etc.
    
    // Colors
    char* start_color;      // Color for start states
    char* accept_color;     // Color for accept states
    char* start_accept_color; // Color for states that are both
    char* default_color;    // Default node color
    
    // Font
    int fontsize;
    char* fontname;
    
    // Metadata
    bool show_metadata;     // Show automaton properties as label
    bool show_alphabet;     // Show alphabet in comment
    bool show_tooltips;     // Show transition counts as tooltips
    
    // Edge styling
    bool merge_edges;       // Merge multiple transitions between same states
    char* edge_color;       // Default edge color
    int edge_fontsize;      // Edge label font size
    
    // Arrow styles for start/accept states
    bool use_start_arrow;   // Add incoming arrow for start states
    bool use_accept_arrow;  // Add outgoing arrow for accept states
    
} fa_styles_dot_style_t;

// Default configuration
#define FA_STYLES_DEFAULT_DOT_STYLE { \
    .rankdir = FA_LAYOUT_LR, \
    .style = FA_STYLE_FILL, \
    .default_shape = "circle", \
    .start_color = "lightyellow", \
    .accept_color = "lightblue", \
    .start_accept_color = "lightgreen", \
    .default_color = "white", \
    .fontsize = 12, \
    .fontname = "Arial", \
    .show_metadata = true, \
    .show_alphabet = true, \
    .show_tooltips = true, \
    .merge_edges = true, \
    .edge_color = "black", \
    .edge_fontsize = 10, \
    .use_start_arrow = false, \
    .use_accept_arrow = false \
}


// Preset configurations
extern const fa_styles_dot_style_t FA_STYLES_DOT_STYLE_CLASSIC;
extern const fa_styles_dot_style_t FA_STYLES_DOT_STYLE_MINIMAL;
extern const fa_styles_dot_style_t FA_STYLES_DOT_STYLE_ARROWS;
extern const fa_styles_dot_style_t FA_STYLES_DOT_STYLE_FANCY;

#endif // FA_STYLES_H