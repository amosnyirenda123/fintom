#ifndef SET_H
#define SET_H

#include <stdbool.h>
#include <stddef.h>

// Function pointer types for type-specific operations
typedef size_t (*HashFunction)(const void*);
typedef bool (*CompareFunction)(const void*, const void*);
typedef void* (*CopyFunction)(const void*);
typedef void (*FreeFunction)(void*);

typedef struct {
    void** members;           // Array of void pointers to elements
    size_t length;           // Current number of elements
    size_t capacity;         // Allocated capacity
    size_t element_size;     // Size of each element in bytes
    CompareFunction compare; // Function to compare two elements
    HashFunction hash;       // Function to hash an element 
    CopyFunction copy;       // Function to copy an element 
    FreeFunction free;       // Function to free an element 
} Set;

// Basic set operations
Set* set_create(size_t element_size, 
                CompareFunction compare,
                HashFunction hash);
Set* set_create_with_functions(size_t element_size,
                              CompareFunction compare,
                              HashFunction hash,
                              CopyFunction copy,
                              FreeFunction free);
void set_destroy(Set* set);
void set_clear(Set* set);

// Element operations
bool set_insert(Set* set, const void* element);
bool set_remove(Set* set, const void* element);
bool set_contains(const Set* set, const void* element);
size_t set_size(const Set* set);
bool set_is_empty(const Set* set);

// Set operations
Set* set_union(const Set* setA, const Set* setB);
Set* set_intersection(const Set* setA, const Set* setB);
Set* set_difference(const Set* setA, const Set* setB);
Set* set_symmetric_difference(const Set* setA, const Set* setB);

// Set relations
bool set_is_subset(const Set* setA, const Set* setB);
bool set_is_superset(const Set* setA, const Set* setB);
bool set_is_equal(const Set* setA, const Set* setB);
bool set_is_disjoint(const Set* setA, const Set* setB);

// Iteration support
typedef struct {
    Set* set;
    size_t index;
} SetIterator;

SetIterator set_iterator_create(Set* set);
bool set_iterator_has_next(const SetIterator* iter);
void* set_iterator_next(SetIterator* iter);
void* set_iterator_current(SetIterator* iter);
void set_iterator_reset(SetIterator* iter);

// Utility functions
Set* set_copy(const Set* set);
bool set_reserve(Set* set, size_t capacity);
double set_load_factor(const Set* set);

// Predefined functions for common types
bool compare_int(const void* a, const void* b);
bool compare_double(const void* a, const void* b);
bool compare_string(const void* a, const void* b);
bool compare_pointer(const void* a, const void* b);

size_t hash_int(const void* a);
size_t hash_double(const void* a);
size_t hash_string(const void* a);
size_t hash_pointer(const void* a);

void* copy_pointer(const void* a);
void free_pointer(void* a);


#ifdef SET_DEBUG_ENABLE
// Debug/print functions, only included if SET_DEBUG_ENABLE is defined
typedef void (*PrintFunction)(const void*);

// Debug print function
void set_print(const Set* set, PrintFunction print_func);

// Example print functions for common types
void print_int(const void* element);
void print_double(const void* element);
void print_string(const void* element);
void print_pointer(const void* element);

// Array conversion functions
void** set_to_array(const Set* set, size_t* size);
void free_set_array(void** array, size_t size, FreeFunction free_func);

#endif // SET_DEBUG_ENABLE


#endif // SET_H