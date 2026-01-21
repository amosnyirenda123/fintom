#include "../../include/set/set.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../../include/common.h"


Set* set_create(size_t element_size, SetCompareFunction compare, SetHashFunction hash) {
    return set_create_with_functions(element_size, compare, hash, 
                                     copy_pointer, free_pointer);
}


Set* set_create_with_functions(size_t element_size,
                               SetCompareFunction compare,
                               SetHashFunction hash,
                               SetCopyFunction copy,
                               SetFreeFunction free) {
    if (element_size == 0 || !compare || !hash) {
        return NULL;
    }
    
    Set* set = malloc(sizeof(Set));
    if (!set) return NULL;
    
    const size_t INITIAL_CAPACITY = 16;
    set->members = malloc(INITIAL_CAPACITY * sizeof(void*));
    if (!set->members) {
        free(set);
        return NULL;
    }
    
    set->length = 0;
    set->capacity = INITIAL_CAPACITY;
    set->element_size = element_size;
    set->compare = compare;
    set->hash = hash;
    set->copy = copy ? copy : copy_pointer;
    set->free = free ? free : free_pointer;
    
    return set;
}

void set_destroy(Set* set) {
    if (!set) return;
    
    set_clear(set);
    free(set->members);
    free(set);
}

void set_clear(Set* set) {
    if (!set) return;
    
    for (size_t i = 0; i < set->length; i++) {
        if (set->members[i]) {
            set->free(set->members[i]);
        }
    }
    set->length = 0;
}


static int find_index(const Set* set, const void* element) {
    if (!set || !element || set->length == 0) return -1;
    
    for (size_t i = 0; i < set->length; i++) {
        if (set->members[i] && set->compare(set->members[i], element)) {
            return i;
        }
    }
    return -1;
}


bool set_insert(Set* set, const void* element) {
    if (!set || !element) return false;
    
    // Check if element already exists
    if (set_contains(set, element)) {
        return false; 
    }
    
    
    if (set->length >= set->capacity) {
        size_t new_capacity = set->capacity * 2;
        void** new_members = realloc(set->members, new_capacity * sizeof(void*));
        if (!new_members) {
            return false;
        }
        set->members = new_members;
        set->capacity = new_capacity;
    }
    
    // Allocate memory for the new element
    void* new_element = malloc(set->element_size);
    if (!new_element) {
        return false;
    }
    
    
    memcpy(new_element, element, set->element_size);
    
    // Apply copy function if it does deep copy
    void* copied_element = set->copy(new_element);
    if (copied_element != new_element) {
        free(new_element);
        new_element = copied_element;
    }
    
    set->members[set->length] = new_element;
    set->length++;
    
    return true;
}

bool set_remove(Set* set, const void* element) {
    if (!set || !element) return false;
    
    int index = find_index(set, element);
    if (index < 0) return false;
    
    // Free the element
    set->free(set->members[index]);
    
    // Shift elements to fill the gap
    for (size_t i = index; i < set->length - 1; i++) {
        set->members[i] = set->members[i + 1];
    }
    
    set->length--;
    set->members[set->length] = NULL; 
    
    return true;
}

bool set_contains(const Set* set, const void* element) {
    return find_index(set, element) >= 0;
}

size_t set_size(const Set* set) {
    return set ? set->length : 0;
}

bool set_is_empty(const Set* set) {
    return !set || set->length == 0;
}

// Set operations
Set* set_union(const Set* setA, const Set* setB) {
    if (!setA && !setB) return NULL;
    if (!setA) return set_copy(setB);
    if (!setB) return set_copy(setA);
    
    // Check compatibility
    if (setA->element_size != setB->element_size ||
        setA->compare != setB->compare ||
        setA->copy != setB->copy ||
        setA->free != setB->free) {
        return NULL;
    }
    
    Set* new_set = set_create_with_functions(
        setA->element_size,
        setA->compare,
        setA->hash,
        setA->copy,
        setA->free
    );
    
    if (!new_set) return NULL;
    
    // Copy all elements from setA
    for (size_t i = 0; i < setA->length; i++) {
        if (!set_insert(new_set, setA->members[i])) {
            set_destroy(new_set);
            return NULL;
        }
    }
    
    // Copy elements from setB that aren't already in new_set
    for (size_t i = 0; i < setB->length; i++) {
        if (!set_contains(setA, setB->members[i])) {
            if (!set_insert(new_set, setB->members[i])) {
                set_destroy(new_set);
                return NULL;
            }
        }
    }
    
    return new_set;
}

Set* set_intersection(const Set* setA, const Set* setB) {
    if (!setA || !setB) {
        // Empty set if either is NULL
        if (setA) return set_create_with_functions(
            setA->element_size, setA->compare, setA->hash, setA->copy, setA->free);
        if (setB) return set_create_with_functions(
            setB->element_size, setB->compare, setB->hash, setB->copy, setB->free);
        return NULL;
    }
    
    // Check compatibility
    if (setA->element_size != setB->element_size ||
        setA->compare != setB->compare) {
        return NULL;
    }
    
    Set* new_set = set_create_with_functions(
        setA->element_size,
        setA->compare,
        setA->hash,
        setA->copy,
        setA->free
    );
    
    if (!new_set) return NULL;
    
    // Insert elements that are in both sets
    for (size_t i = 0; i < setA->length; i++) {
        if (set_contains(setB, setA->members[i])) {
            if (!set_insert(new_set, setA->members[i])) {
                set_destroy(new_set);
                return NULL;
            }
        }
    }
    
    return new_set;
}

Set* set_difference(const Set* setA, const Set* setB) {
    if (!setA) return NULL;
    if (!setB) return set_copy(setA);
    
    // Check compatibility
    if (setA->element_size != setB->element_size ||
        setA->compare != setB->compare) {
        return NULL;
    }
    
    Set* new_set = set_create_with_functions(
        setA->element_size,
        setA->compare,
        setA->hash,
        setA->copy,
        setA->free
    );
    
    if (!new_set) return NULL;
    
    // Insert elements that are in setA but not in setB
    for (size_t i = 0; i < setA->length; i++) {
        if (!set_contains(setB, setA->members[i])) {
            if (!set_insert(new_set, setA->members[i])) {
                set_destroy(new_set);
                return NULL;
            }
        }
    }
    
    return new_set;
}

Set* set_symmetric_difference(const Set* setA, const Set* setB) {
    Set* diff1 = set_difference(setA, setB);
    Set* diff2 = set_difference(setB, setA);
    Set* result = set_union(diff1, diff2);
    
    if (diff1) set_destroy(diff1);
    if (diff2) set_destroy(diff2);
    
    return result;
}

// Set relations
bool set_is_subset(const Set* setA, const Set* setB) {
    if (!setA) return true; // Empty set is subset of any set
    if (!setB) return false;
    
    // Check compatibility
    if (setA->element_size != setB->element_size ||
        setA->compare != setB->compare) {
        return false;
    }
    
    // Check if all elements of setA are in setB
    for (size_t i = 0; i < setA->length; i++) {
        if (!set_contains(setB, setA->members[i])) {
            return false;
        }
    }
    
    return true;
}

bool set_is_superset(const Set* setA, const Set* setB) {
    return set_is_subset(setB, setA);
}

bool set_is_equal(const Set* setA, const Set* setB) {
    if (!setA && !setB) return true;
    if (!setA || !setB) return false;
    
    if (setA->length != setB->length) return false;
    
    return set_is_subset(setA, setB) && set_is_subset(setB, setA);
}

bool set_is_disjoint(const Set* setA, const Set* setB) {
    if (!setA || !setB) return true;
    
    // Check compatibility
    if (setA->element_size != setB->element_size ||
        setA->compare != setB->compare) {
        return false;
    }
    
    // Check if no elements are common
    for (size_t i = 0; i < setA->length; i++) {
        if (set_contains(setB, setA->members[i])) {
            return false;
        }
    }
    
    return true;
}

// Copy function
Set* set_copy(const Set* set) {
    if (!set) return NULL;
    
    Set* new_set = set_create_with_functions(
        set->element_size,
        set->compare,
        set->hash,
        set->copy,
        set->free
    );
    
    if (!new_set) return NULL;
    
    // Reserve capacity
    if (!set_reserve(new_set, set->length)) {
        set_destroy(new_set);
        return NULL;
    }
    
    // Copy all elements
    for (size_t i = 0; i < set->length; i++) {
        if (!set_insert(new_set, set->members[i])) {
            set_destroy(new_set);
            return NULL;
        }
    }
    
    return new_set;
}

// Utility functions
bool set_reserve(Set* set, size_t capacity) {
    if (!set || capacity <= set->capacity) return false;
    
    void** new_members = realloc(set->members, capacity * sizeof(void*));
    if (new_members) {
        set->members = new_members;
        set->capacity = capacity;
    }

    return true;
}

double set_load_factor(const Set* set) {
    if (!set || set->capacity == 0) return 0.0;
    return (double)set->length / (double)set->capacity;
}

// Iterator implementation
SetIterator set_iterator_create(Set* set) {
    SetIterator iter = {set, 0};
    return iter;
}

bool set_iterator_has_next(const SetIterator* iter) {
    return iter && iter->set && iter->index < iter->set->length;
}

void* set_iterator_next(SetIterator* iter) {
    if (!iter || !iter->set || iter->index >= iter->set->length) {
        return NULL;
    }
    return iter->set->members[iter->index++];
}

void* set_iterator_current(SetIterator* iter) {
    if (!iter || !iter->set || iter->index == 0 || 
        iter->index > iter->set->length) {
        return NULL;
    }
    return iter->set->members[iter->index - 1];
}

void set_iterator_reset(SetIterator* iter) {
    if (iter) {
        iter->index = 0;
    }
}


#ifdef SET_DEBUG_ENABLE
#include <stdio.h>

void set_print(const Set* set, PrintFunction print_func) {
    if (!set) {
        printf("(null)\n");
        return;
    }
    
    printf("Set[%zu]: { ", set->length);
    SetIterator iter = set_iterator_create((Set*)set);
    bool first = true;
    
    while (set_iterator_has_next(&iter)) {
        if (!first) printf(", ");
        void* element = set_iterator_next(&iter);
        if (print_func) {
            print_func(element);
        } else {
            printf("%p", element);
        }
        first = false;
    }
    printf(" }\n");
}

void print_int(const void* element) {
    printf("%d", *(const int*)element);
}

void print_double(const void* element) {
    printf("%f", *(const double*)element);
}

void print_string(const void* element) {
    const char* str = *(const char* const*)element;
    printf("\"%s\"", str ? str : "(null)");
}

void print_pointer(const void* element) {
    printf("%p", *(const void* const*)element);
}

void** set_to_array(const Set* set, size_t* size) {
    if (!set) {
        if (size) *size = 0;
        return NULL;
    }
    
    void** array = malloc(set->length * sizeof(void*));
    if (!array) {
        if (size) *size = 0;
        return NULL;
    }
    
    for (size_t i = 0; i < set->length; i++) {
        // Create a copy of each element
        void* element_copy = malloc(set->element_size);
        if (!element_copy) {
            // Cleanup on failure
            for (size_t j = 0; j < i; j++) {
                free(array[j]);
            }
            free(array);
            if (size) *size = 0;
            return NULL;
        }
        memcpy(element_copy, set->members[i], set->element_size);
        array[i] = element_copy;
    }
    
    if (size) *size = set->length;
    return array;
}

void free_set_array(void** array, size_t size, SetFreeFunction free_func) {
    if (!array) return;
    
    for (size_t i = 0; i < size; i++) {
        if (free_func) {
            free_func(array[i]);
        } else {
            free(array[i]);
        }
    }
    free(array);
}

#endif // SET_DEBUG_ENABLE