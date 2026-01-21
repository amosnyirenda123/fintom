#ifndef SET_H
#define SET_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

// Function pointer types for type-specific operations
typedef size_t (*SetHashFunction)(const void*);
typedef bool (*SetCompareFunction)(const void*, const void*);
typedef void* (*SetCopyFunction)(const void*);
typedef void (*SetFreeFunction)(void*);

typedef struct {
    void** members;           // Array of void pointers to elements
    size_t length;           // Current number of elements
    size_t capacity;         // Allocated capacity
    size_t element_size;     // Size of each element in bytes
    SetCompareFunction compare; // Function to compare two elements
    SetHashFunction hash;       // Function to hash an element 
    SetCopyFunction copy;       // Function to copy an element 
    SetFreeFunction free;       // Function to free an element 
} Set;

// Basic set operations
Set* set_create(size_t element_size, 
                SetCompareFunction compare,
                SetHashFunction hash);
Set* set_create_with_functions(size_t element_size,
                              SetCompareFunction compare,
                              SetHashFunction hash,
                              SetCopyFunction copy,
                              SetFreeFunction free);
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




typedef void (*PrintFunction)(const void*);
typedef void (*FprintFunction)(const void*, FILE* file);

void set_print(const Set* set, PrintFunction print_func);
void set_fprint(const Set* set, FprintFunction fprint_func, FILE* file);

void print_int(const void* element);
void print_double(const void* element);
void print_string(const void* element);
void print_pointer(const void* element);
void fprint_int(const void* element, FILE* file);
void fprint_double(const void* element, FILE* file);
void fprint_string(const void* element, FILE* file);
void fprint_pointer(const void* element, FILE* file);


void** set_to_array(const Set* set, size_t* size);
void free_set_array(void** array, size_t size, SetFreeFunction free_func);




#endif // SET_H