#ifndef FA_UTILS_H
#define FA_UTILS_H

#include "set/set.h"

#include <stddef.h>

typedef struct fa_edge_pair {
    char* src;
    char* dest;
    Set* symbols;
} fa_edge_pair_t;

typedef struct fa_edge_map {
    fa_edge_pair_t* pairs;
    size_t size;
    size_t capacity;
} fa_edge_map_t;

fa_edge_map_t* edge_map_create();
void edge_map_destroy(fa_edge_map_t* map);
bool edge_map_add(fa_edge_map_t* map, const char* src, const char* dest, const char* symbol);
void edge_map_write(const fa_edge_map_t* map, FILE* stream);



#endif // FA_UTILS_H