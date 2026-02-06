#include "../include/fa_utils.h"
#include "../include/common.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


fa_edge_map_t* edge_map_create() {
    fa_edge_map_t* map = malloc(sizeof(fa_edge_map_t));
    if (!map) return NULL;
    
    map->capacity = 16;
    map->size = 0;
    map->pairs = malloc(map->capacity * sizeof(fa_edge_pair_t));
    if (!map->pairs) {
        free(map);
        return NULL;
    }
    
    return map;
}


void edge_map_destroy(fa_edge_map_t* map) {
    if (!map) return;
    
    for (size_t i = 0; i < map->size; i++) {
        free(map->pairs[i].src);
        free(map->pairs[i].dest);
        set_destroy(map->pairs[i].symbols);
    }
    free(map->pairs);
    free(map);
}


bool edge_map_add(fa_edge_map_t* map, const char* src, const char* dest, const char* symbol) {
    if (!map || !src || !dest || !symbol) return false;
    
    // Check if pair already exists
    for (size_t i = 0; i < map->size; i++) {
        if (strcmp(map->pairs[i].src, src) == 0 && 
            strcmp(map->pairs[i].dest, dest) == 0) {
            
            char* symbol_copy = strdup(symbol);
            if (!symbol_copy) return false;
            
            bool success = set_insert(map->pairs[i].symbols, &symbol_copy);
            if (!success) free(symbol_copy);
            return success;
        }
    }
    
    // Create new pair
    if (map->size >= map->capacity) {
        size_t new_capacity = map->capacity * 2;
        fa_edge_pair_t* new_pairs = realloc(map->pairs, new_capacity * sizeof(fa_edge_pair_t));
        if (!new_pairs) return false;
        map->pairs = new_pairs;
        map->capacity = new_capacity;
    }
    
    fa_edge_pair_t* pair = &map->pairs[map->size];
    
    // Initialize the new pair
    pair->src = strdup(src);
    pair->dest = strdup(dest);
    pair->symbols = set_create(sizeof(char*), compare_strings, hash_string);
    
    if (!pair->src || !pair->dest || !pair->symbols) {
        // Cleanup on failure
        free(pair->src);
        free(pair->dest);
        if (pair->symbols) set_destroy(pair->symbols);
        return false;
    }
    
    // Add the symbol
    char* symbol_copy = strdup(symbol);
    if (!symbol_copy) {
        free(pair->src);
        free(pair->dest);
        set_destroy(pair->symbols);
        return false;
    }
    
    if (!set_insert(pair->symbols, &symbol_copy)) {
        free(pair->src);
        free(pair->dest);
        set_destroy(pair->symbols);
        free(symbol_copy);
        return false;
    }
    
    map->size++;
    return true;
}

void edge_map_write(const fa_edge_map_t* map, FILE* stream) {
    if (!map || !stream) return;
    
    for (size_t i = 0; i < map->size; i++) {
        const fa_edge_pair_t* pair = &map->pairs[i];
        
        // Check if set has elements
        if (pair->symbols->length == 0) continue;
        
        // Build label string
        char combined[1024] = "";
        bool first = true;
        
        // Iterate through set
        SetIterator iter = set_iterator_create((Set*)pair->symbols);
        while (set_iterator_has_next(&iter)) {
            if (!first) {
                strcat(combined, ", ");
            }
            void* element = set_iterator_next(&iter);
            strcat(combined, *(const char* const*)element);
            first = false;
        }
        
        fprintf(stream, "  \"%s\" -> \"%s\" [label=\"%s\"];\n", 
                pair->src, pair->dest, combined);
    }
}