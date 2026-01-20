#include "../../include/hash/hash_table.h"


typedef struct HashNode {
    void* key;
    void* value;
    struct HashNode* next;
    uint32_t hash;  // Store hash for faster resizing
} HashNode;

struct HashTable {
    HashNode** nodes;
    size_t node_count;
    size_t element_count;
    
    // Type information
    size_t key_size;
    size_t value_size;
    
    // Function pointers
    HashFunction hash_func;
    CompareFunction key_compare_func;
    KeyCopyFunction key_copy_func;
    KeyFreeFunction key_free_func;
    ValueCopyFunction value_copy_func;
    ValueFreeFunction value_free_func;
    
    // Configuration
    double max_load_factor;
    size_t resize_threshold;
};


#define DEFAULT_CAPACITY 16
#define DEFAULT_MAX_LOAD_FACTOR 0.75
#define MIN_CAPACITY 8


static const size_t primes[] = {
    53, 97, 193, 389, 769, 1543, 3079, 6151, 
    12289, 24593, 49157, 98317, 196613, 393241, 
    786433, 1572869, 3145739, 6291469, 12582917, 
    25165843, 50331653, 100663319, 201326611, 
    402653189, 805306457, 1610612741
};


static size_t next_prime(size_t n) {
    for (size_t i = 0; i < sizeof(primes)/sizeof(primes[0]); i++) {
        if (primes[i] >= n) return primes[i];
    }
    // Fallback to power of two if primes exhausted
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    #if SIZE_MAX > 0xFFFFFFFF
    n |= n >> 32;
    #endif
    return n + 1;
}

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
) {
    if (!hash_func || !key_compare_func) return NULL;
    
    HashTable* table = malloc(sizeof(HashTable));
    if (!table) return NULL;
    
    if (initial_capacity < MIN_CAPACITY) {
        initial_capacity = DEFAULT_CAPACITY;
    }
    
    size_t capacity = next_prime(initial_capacity);
    
    table->nodes = calloc(capacity, sizeof(HashNode*));
    if (!table->nodes) {
        free(table);
        return NULL;
    }
    
    table->node_count = capacity;
    table->element_count = 0;
    table->key_size = key_size;
    table->value_size = value_size;
    table->hash_func = hash_func;
    table->key_compare_func = key_compare_func;
    table->key_copy_func = key_copy_func ? key_copy_func : copy_pointer;
    table->key_free_func = key_free_func ? key_free_func : free_pointer;
    table->value_copy_func = value_copy_func ? value_copy_func : copy_pointer;
    table->value_free_func = value_free_func ? value_free_func : free_pointer;
    table->max_load_factor = DEFAULT_MAX_LOAD_FACTOR;
    table->resize_threshold = (size_t)(capacity * DEFAULT_MAX_LOAD_FACTOR);
    
    return table;
}

HashTable* hash_table_create_string_string(size_t initial_capacity) {
    return hash_table_create(
        sizeof(char*), sizeof(char*),
        initial_capacity,
        hash_string,
        compare_strings,
        copy_string,
        free_string,
        copy_string,
        free_string
    );
}


static void resize_if_needed(HashTable* table) {
    if (table->element_count >= table->resize_threshold) {
        size_t new_capacity = table->node_count * 2;
        hash_table_rehash(table, new_capacity);
    }
}


bool hash_table_insert(HashTable* table, const void* key, const void* value) {
    if (!table || !key) return false;
    
    resize_if_needed(table);
    
    uint32_t hash = table->hash_func(key, table->node_count);
    size_t index = hash % table->node_count;
    
    // Check if key already exists
    HashNode* current = table->nodes[index];
    while (current) {
        if (current->hash == hash && 
            table->key_compare_func(current->key, key)) {
            // Key exists, update value
            table->value_free_func(current->value);
            current->value = table->value_copy_func(value);
            return true;
        }
        current = current->next;
    }
    
    // Create new node
    HashNode* new_node = malloc(sizeof(HashNode));
    if (!new_node) return false;
    
    new_node->key = table->key_copy_func(key);
    new_node->value = table->value_copy_func(value);
    new_node->hash = hash;
    new_node->next = table->nodes[index];
    table->nodes[index] = new_node;
    table->element_count++;
    
    return true;
}


void* hash_table_get(const HashTable* table, const void* key) {
    if (!table || !key) return NULL;
    
    uint32_t hash = table->hash_func(key, table->node_count);
    size_t index = hash % table->node_count;
    
    HashNode* current = table->nodes[index];
    while (current) {
        if (current->hash == hash && 
            table->key_compare_func(current->key, key)) {
            return current->value;
        }
        current = current->next;
    }
    
    return NULL;
}


bool hash_table_remove(HashTable* table, const void* key){

}

bool hash_table_contains(const HashTable* table, const void* key){

}

void hash_table_destroy(HashTable* table){

}
void hash_table_clear(HashTable* table){

}


size_t hash_table_size(const HashTable* table){

}
size_t hash_table_capacity(const HashTable* table){

}
bool hash_table_is_empty(const HashTable* table){

}
double hash_table_load_factor(const HashTable* table){

}


//Iterators
HashTableIterator hash_table_iterator_create(HashTable* table){

}
bool hash_table_iterator_has_next(const HashTableIterator* iter){

}
bool hash_table_iterator_next(HashTableIterator* iter, void* key_out, void* value_out){

}
void hash_table_iterator_reset(HashTableIterator* iter){

}

bool hash_table_reserve(HashTable* table, size_t new_capacity)
{

}
bool hash_table_rehash(HashTable* table, size_t new_capacity)
{

}
HashTable* hash_table_copy(const HashTable* table)
{
    
}


//default hash functions

size_t hash_string(const void* key, size_t table_size) {
    const char* str = *(const char* const*)key;
    size_t hash = 5381;
    int c;
    
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    
    return hash % table_size;
}


bool compare_strings(const void* key1, const void* key2) {
    const char* str1 = *(const char* const*)key1;
    const char* str2 = *(const char* const*)key2;
    
    if (str1 == str2) return true;
    if (!str1 || !str2) return false;
    return strcmp(str1, str2) == 0;
}

void* copy_string(const void* str) {
    const char* original = *(const char* const*)str;
    if (!original) return NULL;
    return strdup(original);
}

void free_string(void* str) {
    free(*(char**)str);
}





#ifdef HASH_TABLE_DEBUG

// ==================== Print Functions ====================

void hash_table_print_string_key(const void* key) {
    const char* str = *(const char* const*)key;
    printf("\"%s\"", str ? str : "(null)");
}

void hash_table_print_int_key(const void* key) {
    printf("%d", *(const int*)key);
}

void hash_table_print_double_key(const void* key) {
    printf("%.2f", *(const double*)key);
}

void hash_table_print_string_value(const void* value) {
    const char* str = *(const char* const*)value;
    printf("\"%s\"", str ? str : "(null)");
}

void hash_table_print_int_value(const void* value) {
    printf("%d", *(const int*)value);
}

void hash_table_print_double_value(const void* value) {
    printf("%.2f", *(const double*)value);
}

void hash_table_print_pointer_value(const void* value) {
    printf("%p", *(const void* const*)value);
}

void hash_table_print(const HashTable* table, 
                      KeyPrintFunction print_key,
                      ValuePrintFunction print_value) {
    if (!table) {
        printf("HashTable: (null)\n");
        return;
    }
    
    printf("\n=== HashTable Debug Print ===\n");
    printf("Elements: %zu, Buckets: %zu, Load Factor: %.2f%%\n",
           table->element_count, table->bucket_count,
           hash_table_load_factor(table) * 100);
    printf("Key size: %zu bytes, Value size: %zu bytes\n",
           table->key_size, table->value_size);
    printf("Max Load Factor: %.2f\n", table->max_load_factor);
    printf("Resize Threshold: %zu\n", table->resize_threshold);
    printf("\nBucket Contents:\n");
    printf("========================================\n");
    
    for (size_t i = 0; i < table->bucket_count; i++) {
        HashNode* node = table->buckets[i];
        if (node) {
            printf("[%04zu]: ", i);
            while (node) {
                printf("(");
                if (print_key) {
                    print_key(node->key);
                } else {
                    printf("%p", node->key);
                }
                printf(" -> ");
                if (print_value) {
                    print_value(node->value);
                } else {
                    printf("%p", node->value);
                }
                printf(")");
                
                if (node->next) {
                    printf(" -> ");
                }
                node = node->next;
            }
            printf("\n");
        }
    }
    
    if (table->element_count == 0) {
        printf("(empty)\n");
    }
    printf("========================================\n\n");
}

void hash_table_print_detailed_stats(const HashTable* table) {
    if (!table) return;
    
    HashTableStats stats = hash_table_get_stats(table);
    
    printf("\n=== HashTable Detailed Statistics ===\n");
    printf("Total Elements: %zu\n", stats.element_count);
    printf("Total Buckets: %zu\n", stats.bucket_count);
    printf("Load Factor: %.2f%%\n", stats.load_factor * 100);
    printf("Max Bucket Length: %zu\n", stats.max_bucket_length);
    printf("Min Bucket Length: %zu\n", stats.min_bucket_length);
    printf("Average Bucket Length: %.2f\n", stats.average_bucket_length);
    
    // Calculate empty buckets
    size_t empty_buckets = 0;
    for (size_t i = 0; i < table->bucket_count; i++) {
        if (table->buckets[i] == NULL) {
            empty_buckets++;
        }
    }
    
    printf("Empty Buckets: %zu (%.1f%%)\n", empty_buckets,
           (double)empty_buckets / table->bucket_count * 100);
    
    // Bucket length distribution
    printf("\nBucket Length Distribution:\n");
    size_t max_len = stats.max_bucket_length;
    size_t* distribution = calloc(max_len + 1, sizeof(size_t));
    
    if (distribution) {
        for (size_t i = 0; i < table->bucket_count; i++) {
            size_t len = 0;
            HashNode* node = table->buckets[i];
            while (node) {
                len++;
                node = node->next;
            }
            distribution[len]++;
        }
        
        for (size_t i = 0; i <= max_len; i++) {
            if (distribution[i] > 0) {
                printf("  Length %zu: %zu buckets (%.1f%%)\n", 
                       i, distribution[i],
                       (double)distribution[i] / table->bucket_count * 100);
            }
        }
        free(distribution);
    }
    printf("\n");
}

void hash_table_print_bucket_distribution(const HashTable* table) {
    if (!table) return;
    
    printf("\n=== HashTable Bucket Distribution ===\n");
    
    // Create a histogram
    size_t max_len = 0;
    for (size_t i = 0; i < table->bucket_count; i++) {
        size_t len = 0;
        HashNode* node = table->buckets[i];
        while (node) {
            len++;
            node = node->next;
        }
        if (len > max_len) max_len = len;
    }
    
    size_t* histogram = calloc(max_len + 1, sizeof(size_t));
    if (!histogram) return;
    
    for (size_t i = 0; i < table->bucket_count; i++) {
        size_t len = 0;
        HashNode* node = table->buckets[i];
        while (node) {
            len++;
            node = node->next;
        }
        histogram[len]++;
    }
    
    // Print histogram
    printf("Bucket Length | Count | Visual\n");
    printf("--------------|-------|--------\n");
    
    size_t max_count = 0;
    for (size_t i = 0; i <= max_len; i++) {
        if (histogram[i] > max_count) max_count = histogram[i];
    }
    
    const int BAR_WIDTH = 40;
    for (size_t i = 0; i <= max_len; i++) {
        if (histogram[i] > 0) {
            int bar_length = (int)((double)histogram[i] / max_count * BAR_WIDTH);
            printf("%13zu | %5zu | ", i, histogram[i]);
            for (int j = 0; j < bar_length; j++) {
                printf("█");
            }
            printf("\n");
        }
    }
    
    free(histogram);
    printf("\n");
}

bool hash_table_validate(const HashTable* table) {
    if (!table) {
        printf("ERROR: Table is NULL\n");
        return false;
    }
    
    if (!table->buckets) {
        printf("ERROR: Buckets array is NULL\n");
        return false;
    }
    
    if (table->bucket_count == 0) {
        printf("ERROR: Bucket count is zero\n");
        return false;
    }
    
    if (table->max_load_factor <= 0 || table->max_load_factor > 1.0) {
        printf("ERROR: Invalid max load factor: %.2f\n", table->max_load_factor);
        return false;
    }
    
    // Count elements
    size_t counted_elements = 0;
    for (size_t i = 0; i < table->bucket_count; i++) {
        HashNode* node = table->buckets[i];
        while (node) {
            counted_elements++;
            
            // Validate hash
            uint32_t computed_hash = table->hash_func(node->key, table->bucket_count);
            size_t computed_index = computed_hash % table->bucket_count;
            
            if (computed_index != i) {
                printf("ERROR: Node in wrong bucket! Hash: %u, Expected index: %zu, Actual index: %zu\n",
                       node->hash, computed_index, i);
                return false;
            }
            
            // Check for duplicate keys in same bucket (shouldn't happen)
            HashNode* check = node->next;
            while (check) {
                if (table->key_compare_func(node->key, check->key)) {
                    printf("ERROR: Duplicate key found in bucket %zu\n", i);
                    return false;
                }
                check = check->next;
            }
            
            node = node->next;
        }
    }
    
    if (counted_elements != table->element_count) {
        printf("ERROR: Element count mismatch! Counted: %zu, Reported: %zu\n",
               counted_elements, table->element_count);
        return false;
    }
    
    printf("Hash table validation PASSED\n");
    return true;
}

bool hash_table_dump_to_file(const HashTable* table, 
                            const char* filename,
                            KeyPrintFunction print_key,
                            ValuePrintFunction print_value) {
    if (!table || !filename) return false;
    
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("ERROR: Could not open file '%s' for writing\n", filename);
        return false;
    }
    
    fprintf(file, "Hash Table Dump\n");
    fprintf(file, "================\n\n");
    fprintf(file, "Configuration:\n");
    fprintf(file, "  Elements: %zu\n", table->element_count);
    fprintf(file, "  Buckets: %zu\n", table->bucket_count);
    fprintf(file, "  Load Factor: %.2f%%\n", hash_table_load_factor(table) * 100);
    fprintf(file, "  Key Size: %zu bytes\n", table->key_size);
    fprintf(file, "  Value Size: %zu bytes\n", table->value_size);
    fprintf(file, "\n");
    
    fprintf(file, "Contents:\n");
    fprintf(file, "---------\n");
    
    for (size_t i = 0; i < table->bucket_count; i++) {
        HashNode* node = table->buckets[i];
        if (node) {
            fprintf(file, "Bucket [%04zu]:\n", i);
            int item_num = 0;
            while (node) {
                fprintf(file, "  %d. Key: ", ++item_num);
                
                // Write key
                if (print_key) {
                    // We need to capture the printed output
                    // This is tricky without modifying print functions
                    // For simplicity, we'll handle common types
                    if (print_key == hash_table_print_string_key) {
                        const char* str = *(const char* const*)node->key;
                        fprintf(file, "\"%s\"", str ? str : "(null)");
                    } else if (print_key == hash_table_print_int_key) {
                        fprintf(file, "%d", *(const int*)node->key);
                    } else {
                        fprintf(file, "%p", node->key);
                    }
                } else {
                    fprintf(file, "%p", node->key);
                }
                
                fprintf(file, " -> Value: ");
                
                // Write value
                if (print_value) {
                    if (print_value == hash_table_print_string_value) {
                        const char* str = *(const char* const*)node->value;
                        fprintf(file, "\"%s\"", str ? str : "(null)");
                    } else if (print_value == hash_table_print_int_value) {
                        fprintf(file, "%d", *(const int*)node->value);
                    } else {
                        fprintf(file, "%p", node->value);
                    }
                } else {
                    fprintf(file, "%p", node->value);
                }
                
                fprintf(file, " (hash: %u)\n", node->hash);
                node = node->next;
            }
        }
    }
    
    fclose(file);
    printf("Hash table dumped to '%s'\n", filename);
    return true;
}

// ==================== Debug Iterator ====================

DebugHashTableIterator hash_table_debug_iterator_create(
    HashTable* table,
    KeyPrintFunction print_key,
    ValuePrintFunction print_value) {
    
    DebugHashTableIterator iter;
    iter.base = hash_table_iterator_create(table);
    iter.print_key = print_key;
    iter.print_value = print_value;
    return iter;
}

bool hash_table_debug_iterator_next(DebugHashTableIterator* iter) {
    if (!iter) return false;
    
    void* key;
    void* value;
    bool has_next = hash_table_iterator_next(&iter->base, &key, &value);
    
    if (has_next) {
        printf("Iterating: ");
        if (iter->print_key) {
            iter->print_key(key);
        } else {
            printf("%p", key);
        }
        printf(" -> ");
        if (iter->print_value) {
            iter->print_value(value);
        } else {
            printf("%p", value);
        }
        printf("\n");
    }
    
    return has_next;
}

// ==================== Memory Statistics ====================

HashTableMemoryStats hash_table_get_memory_stats(const HashTable* table) {
    HashTableMemoryStats stats = {0};
    
    if (!table) return stats;
    
    // Table structure
    stats.table_struct_size = sizeof(HashTable);
    
    // Buckets array
    stats.buckets_memory = table->bucket_count * sizeof(HashNode*);
    
    // Nodes and their contents
    for (size_t i = 0; i < table->bucket_count; i++) {
        HashNode* node = table->buckets[i];
        while (node) {
            stats.nodes_memory += sizeof(HashNode);
            
            // Estimate key/value memory (exact depends on copy functions)
            if (table->key_size > 0) {
                stats.keys_memory += table->key_size;
            }
            if (table->value_size > 0) {
                stats.values_memory += table->value_size;
            }
            
            node = node->next;
        }
    }
    
    stats.total_memory = stats.table_struct_size + stats.buckets_memory +
                        stats.nodes_memory + stats.keys_memory + 
                        stats.values_memory;
    
    return stats;
}

void hash_table_print_memory_stats(const HashTable* table) {
    if (!table) return;
    
    HashTableMemoryStats stats = hash_table_get_memory_stats(table);
    
    printf("\n=== HashTable Memory Statistics ===\n");
    printf("Table Structure: %zu bytes\n", stats.table_struct_size);
    printf("Buckets Array:   %zu bytes\n", stats.buckets_memory);
    printf("Nodes:           %zu bytes\n", stats.nodes_memory);
    printf("Keys:           ~%zu bytes\n", stats.keys_memory);
    printf("Values:         ~%zu bytes\n", stats.values_memory);
    printf("-------------------------------\n");
    printf("TOTAL:          ~%zu bytes\n", stats.total_memory);
    printf("                 (~%.1f KB)\n", stats.total_memory / 1024.0);
    
    if (table->element_count > 0) {
        printf("\nPer Element:     ~%.1f bytes\n", 
               (double)stats.total_memory / table->element_count);
    }
    printf("\n");
}

// ==================== Performance Counters ====================

#ifdef HASH_TABLE_PERF_COUNTERS
// Add these fields to HashTable struct:
// HashTablePerfCounters perf_counters;

HashTablePerfCounters hash_table_get_perf_counters(const HashTable* table) {
    if (!table) {
        HashTablePerfCounters empty = {0};
        return empty;
    }
    return table->perf_counters;
}

void hash_table_reset_perf_counters(HashTable* table) {
    if (table) {
        memset(&table->perf_counters, 0, sizeof(HashTablePerfCounters));
    }
}

void hash_table_print_perf_stats(const HashTable* table) {
    if (!table) return;
    
    HashTablePerfCounters perf = table->perf_counters;
    
    printf("\n=== HashTable Performance Statistics ===\n");
    printf("Insert Operations: %zu\n", perf.insert_count);
    if (perf.insert_count > 0) {
        printf("  Collisions: %zu (%.1f%%)\n", perf.insert_collisions,
               (double)perf.insert_collisions / perf.insert_count * 100);
    }
    
    printf("Search Operations: %zu\n", perf.search_count);
    if (perf.search_count > 0) {
        printf("  Avg Steps per Search: %.2f\n", 
               (double)perf.search_steps / perf.search_count);
    }
    
    printf("Delete Operations: %zu\n", perf.delete_count);
    printf("Rehash Operations: %zu\n", perf.rehash_count);
    
    if (perf.insert_count + perf.search_count + perf.delete_count > 0) {
        size_t total_ops = perf.insert_count + perf.search_count + perf.delete_count;
        printf("\nTotal Operations: %zu\n", total_ops);
    }
    printf("\n");
}
#endif // HASH_TABLE_PERF_COUNTERS

#endif // HASH_TABLE_DEBUG