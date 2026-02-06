#include "../include/fa_styles.h"
#include <stdbool.h>



// Dot File Styles
const fa_styles_dot_style_t FA_STYLES_DOT_STYLE_CLASSIC = {
    .rankdir = FA_LAYOUT_LR,
    .style = FA_STYLE_FILL,
    .default_shape = "circle",
    .start_color = "lightyellow",
    .accept_color = "lightblue",
    .start_accept_color = "lightgreen",
    .default_color = "white",
    .fontsize = 12,
    .fontname = "Arial",
    .show_metadata = true,
    .show_alphabet = true,
    .show_tooltips = true,
    .merge_edges = true,
    .edge_color = "black",
    .edge_fontsize = 10,
    .use_start_arrow = false,
    .use_accept_arrow = false
};

const fa_styles_dot_style_t FA_STYLES_DOT_STYLE_MINIMAL = {
    .rankdir = FA_LAYOUT_LR,
    .style = FA_STYLE_MINIMAL,
    .default_shape = "circle",
    .start_color = "white",
    .accept_color = "white",
    .start_accept_color = "white",
    .default_color = "white",
    .fontsize = 10,
    .fontname = "Helvetica",
    .show_metadata = false,
    .show_alphabet = false,
    .show_tooltips = false,
    .merge_edges = false,
    .edge_color = "black",
    .edge_fontsize = 8,
    .use_start_arrow = false,
    .use_accept_arrow = false
};

const fa_styles_dot_style_t FA_STYLES_DOT_STYLE_ARROWS = {
    .rankdir = FA_LAYOUT_LR,
    .style = FA_STYLE_MINIMAL,  // No fill colors
    .default_shape = "circle",
    .start_color = "white",
    .accept_color = "white",
    .start_accept_color = "white",
    .default_color = "white",
    .fontsize = 12,
    .fontname = "Arial",
    .show_metadata = true,
    .show_alphabet = true,
    .show_tooltips = true,
    .merge_edges = true,
    .edge_color = "black",
    .edge_fontsize = 10,
    .use_start_arrow = true,    // Use arrows for start states
    .use_accept_arrow = true    // Use arrows for accept states
};

const fa_styles_dot_style_t FA_STYLES_DOT_STYLE_FANCY = {
    .rankdir = FA_LAYOUT_TB,
    .style = FA_STYLE_PERIPHERY,
    .default_shape = "ellipse",
    .start_color = "gold",
    .accept_color = "lightblue",
    .start_accept_color = "palegreen",
    .default_color = "ghostwhite",
    .fontsize = 14,
    .fontname = "Times-Roman",
    .show_metadata = true,
    .show_alphabet = true,
    .show_tooltips = true,
    .merge_edges = true,
    .edge_color = "navy",
    .edge_fontsize = 11,
    .use_start_arrow = false,
    .use_accept_arrow = false
};