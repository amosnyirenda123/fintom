#ifndef HASH_TABLE_H
#define HASH_TABLE_H
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Function pointer types for type-specific operations
typedef size_t (*HashFunction)(const void* key, size_t table_size);
typedef bool (*CompareFunction)(const void* key1, const void* key2);
typedef void* (*KeyCopyFunction)(const void* key);
typedef void (*KeyFreeFunction)(void* key);
typedef void* (*ValueCopyFunction)(const void* value);
typedef void (*ValueFreeFunction)(void* value);

// Iterator structure
typedef struct HashTableIterator {
    struct HashTable* table;
    size_t bucket_index;
    struct HashNode* current_node;
} HashTableIterator;

// Main hash table structure (opaque pointer)
typedef struct HashTable HashTable;


HashTable* hash_table_create(
    size_t key_size,
    size_t value_size,
    size_t initial_capacity,
    HashFunction hash_func,
    CompareFunction key_compare_func,
    KeyCopyFunction key_copy_func,
    KeyFreeFunction key_free_func,
    ValueCopyFunction value_copy_func,
    ValueFreeFunction value_free_func
);


HashTable* hash_table_create_string_string(size_t initial_capacity);
HashTable* hash_table_create_string_int(size_t initial_capacity);
HashTable* hash_table_create_int_string(size_t initial_capacity);
HashTable* hash_table_create_int_int(size_t initial_capacity);

void hash_table_destroy(HashTable* table);
void hash_table_clear(HashTable* table);


bool hash_table_insert(HashTable* table, const void* key, const void* value);
//bool hash_table_upsert(HashTable* table, const void* key, const void* value);
bool hash_table_remove(HashTable* table, const void* key);
void* hash_table_get(const HashTable* table, const void* key);
bool hash_table_contains(const HashTable* table, const void* key);

// ==================== Table Properties ====================
size_t hash_table_size(const HashTable* table);
size_t hash_table_capacity(const HashTable* table);
bool hash_table_is_empty(const HashTable* table);
double hash_table_load_factor(const HashTable* table);

// ==================== Iteration ====================
HashTableIterator hash_table_iterator_create(HashTable* table);
bool hash_table_iterator_has_next(const HashTableIterator* iter);
bool hash_table_iterator_next(HashTableIterator* iter, void* key_out, void* value_out);
void hash_table_iterator_reset(HashTableIterator* iter);

// ==================== Table Operations ====================
bool hash_table_reserve(HashTable* table, size_t new_capacity);
bool hash_table_rehash(HashTable* table, size_t new_capacity);
HashTable* hash_table_copy(const HashTable* table);

// ==================== Serialization ====================
//bool hash_table_save_to_file(const HashTable* table, const char* filename);
//HashTable* hash_table_load_from_file(const char* filename,
                                    // size_t key_size,
                                    // size_t value_size,
                                    // HashFunction hash_func,
                                    // CompareFunction key_compare_func,
                                    // KeyCopyFunction key_copy_func,
                                    // KeyFreeFunction key_free_func,
                                    // ValueCopyFunction value_copy_func,
                                    // ValueFreeFunction value_free_func);



// ==================== Utility Functions ====================


// Debug functions
#ifdef HASH_TABLE_DEBUG
#include <stdio.h>


// Print function types
typedef void (*KeyPrintFunction)(const void* key);
typedef void (*ValuePrintFunction)(const void* value);

// Print the entire hash table
void hash_table_print(const HashTable* table, 
                      KeyPrintFunction print_key,
                      ValuePrintFunction print_value);

// Print statistics in detail
void hash_table_print_detailed_stats(const HashTable* table);

// Print bucket distribution
void hash_table_print_bucket_distribution(const HashTable* table);

// Validate hash table integrity
bool hash_table_validate(const HashTable* table);

// Dump to file in human-readable format
bool hash_table_dump_to_file(const HashTable* table, 
                            const char* filename,
                            KeyPrintFunction print_key,
                            ValuePrintFunction print_value);

// Predefined print functions
void hash_table_print_string_key(const void* key);
void hash_table_print_int_key(const void* key);
void hash_table_print_double_key(const void* key);
void hash_table_print_string_value(const void* value);
void hash_table_print_int_value(const void* value);
void hash_table_print_double_value(const void* value);
void hash_table_print_pointer_value(const void* value);

// Debug iterator that prints while iterating
typedef struct {
    HashTableIterator base;
    KeyPrintFunction print_key;
    ValuePrintFunction print_value;
} DebugHashTableIterator;

DebugHashTableIterator hash_table_debug_iterator_create(
    HashTable* table,
    KeyPrintFunction print_key,
    ValuePrintFunction print_value);
    
bool hash_table_debug_iterator_next(DebugHashTableIterator* iter);

// Memory usage statistics
typedef struct {
    size_t table_struct_size;
    size_t buckets_memory;
    size_t nodes_memory;
    size_t keys_memory;
    size_t values_memory;
    size_t total_memory;
} HashTableMemoryStats;

HashTableMemoryStats hash_table_get_memory_stats(const HashTable* table);
void hash_table_print_memory_stats(const HashTable* table);

// Performance counters 
#ifdef HASH_TABLE_PERF_COUNTERS
typedef struct {
    size_t insert_count;
    size_t insert_collisions;
    size_t search_count;
    size_t search_steps;
    size_t delete_count;
    size_t rehash_count;
} HashTablePerfCounters;

HashTablePerfCounters hash_table_get_perf_counters(const HashTable* table);
void hash_table_reset_perf_counters(HashTable* table);
void hash_table_print_perf_stats(const HashTable* table);
#endif

#endif // HASH_TABLE_DEBUG

#endif // HASH_TABLE_H