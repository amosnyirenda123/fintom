#include "../include/common.h"
#include <stdlib.h>
#include <string.h>




//Hash functions
size_t hash_string(const void* a) {
    const char* str = *(const char* const*)a;
    if (!str) return 0;
    
    size_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

size_t hash_int(const void* a){
    int value = *(const int*)a;
    return *(size_t*)&value;
}

size_t hash_double(const void* a) {
    double value = *(const double*)a;
    return *(size_t*)&value;
}

size_t hash_pointer(const void* a) {
    return (size_t)a;
}

size_t hash_pointer_v2(const void* key, size_t table_size) {
    void* ptr = *(void* const*)key;
    return ((uintptr_t)ptr) % table_size;
}

size_t hash_string_v2(const void* key, size_t table_size) {
    
    const char* str = *(const char* const*)key;
    
    if (!str) return 0; 
    
    // DJB2 hash algorithm
    unsigned long hash = 5381;
    int c;
    
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;  // hash * 33 + c
    }
    
    return hash % table_size;
}


size_t hash_int_v2(const void* key, size_t table_size) {
    int value = *(const int*)key;
    
    // Knuth's multiplicative method
    uint64_t hash = (uint64_t)value * 2654435761ULL;
    
    // Mix bits
    hash ^= hash >> 33;
    hash *= 0xff51afd7ed558ccdULL;
    hash ^= hash >> 33;
    hash *= 0xc4ceb9fe1a85ec53ULL;
    hash ^= hash >> 33;
    
    return hash % table_size;
}





//Compare

bool compare_pointers(const void* a, const void* b) {
    return a == b;
}

bool compare_ints(const void* a, const void* b) {
    return *(const int*)a == *(const int*)b;
}

bool compare_doubles(const void* a, const void* b) {
    return *(const double*)a == *(const double*)b;
}

bool compare_strings(const void* a, const void* b) {
    const char* str_a = *(const char* const*)a;
    const char* str_b = *(const char* const*)b;
    if (str_a == str_b) return true;
    if (!str_a || !str_b) return false;
    return strcmp(str_a, str_b) == 0;
}



//Copy

void* copy_string(const void* str) {
    const char* original = *(const char* const*)str;
    if (!original) return NULL;
    return strdup(original);
}

void* copy_int(const void* num) {
    int* copy = malloc(sizeof(int));
    if (copy) {
        *copy = *(const int*)num;
    }
    return copy;
}

void* copy_pointer(const void* ptr) {
    void** copy = malloc(sizeof(void*));
    if (copy) {
        *copy = *(void* const*)ptr;
    }
    return copy;
}


//free functions
void free_string(void* str) {
    free(*(char**)str);
}

void free_int(void* num) {
    free(num);
}
void free_pointer(void* a) {
    
}


int exists_in_array(const int arr[], const int size, int const value) {
    for (int i = 0; i < size; i++) {
        if (arr[i] == value) {
            return 1;
        }
    }
    return 0;
}










