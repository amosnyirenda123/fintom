#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


bool compare_ints(const void* a, const void* b);
bool compare_doubles(const void* a, const void* b);
bool compare_strings(const void* a, const void* b);
bool compare_pointers(const void* a, const void* b);

size_t hash_int(const void* a);
size_t hash_double(const void* a);
size_t hash_string(const void* a);
size_t hash_pointer(const void* a);
size_t hash_int_v2(const void* key, size_t table_size);
size_t hash_string_v2(const void* key, size_t table_size) ;
size_t hash_pointer_v2(const void* key, size_t table_size);

void* copy_string(const void* str);
void* copy_pointer(const void* a);
void* copy_int(const void* num);

void free_string(void* str);
void free_int(void* num);
void free_pointer(void* a);

int exists_in_array(const int arr[], const int size, int const value);


#endif // COMMON_H