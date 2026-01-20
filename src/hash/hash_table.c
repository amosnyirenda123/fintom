#include "../../include/hash/hash_table.h"
#include "../../include/common.h"
#include <string.h>




typedef struct HashNode {
    void* key;
    void* value;
    struct HashNode* next;
    uint32_t hash;  //hash for faster resizing
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


// Helper Functions

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


static void resize_if_needed(HashTable* table) {
    if (table->element_count >= table->resize_threshold) {
        size_t new_capacity = table->node_count * 2;
        hash_table_rehash(table, new_capacity);
    }
}



//Constructors
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
        hash_string_v2,
        compare_strings,
        copy_string,
        free_string,
        copy_string,
        free_string
    );
}


HashTable* hash_table_create_string_int(size_t initial_capacity) {
    return hash_table_create(
        sizeof(char*),       // key_size
        sizeof(int),         // value_size
        initial_capacity,
        hash_string_v2,         // hash_func for strings
        compare_strings,     // key_compare_func for strings
        copy_string,         // key_copy_func for strings
        free_string,         // key_free_func for strings
        copy_int,            // value_copy_func for ints
        free_int             // value_free_func for ints
    );
}

HashTable* hash_table_create_int_string(size_t initial_capacity) {
    return hash_table_create(
        sizeof(int),         // key_size
        sizeof(char*),       // value_size
        initial_capacity,
        hash_int_v2,            // hash_func for ints
        compare_ints,        // key_compare_func for ints
        copy_int,            // key_copy_func for ints
        free_int,            // key_free_func for ints
        copy_string,         // value_copy_func for strings
        free_string          // value_free_func for strings
    );
}

HashTable* hash_table_create_int_int(size_t initial_capacity) {
    return hash_table_create(
        sizeof(int),         // key_size
        sizeof(int),         // value_size
        initial_capacity,
        hash_int_v2,            // hash_func for ints
        compare_ints,        // key_compare_func for ints
        copy_int,            // key_copy_func for ints
        free_int,            // key_free_func for ints
        copy_int,            // value_copy_func for ints
        free_int             // value_free_func for ints
    );
}


HashTable* hash_table_create_string_ptr(size_t initial_capacity) {
    return hash_table_create(
        sizeof(char*),
        sizeof(void*),
        initial_capacity,
        hash_string_v2,
        compare_strings,
        copy_string,
        free_string,
        copy_pointer,
        free_pointer
    );
}

HashTable* hash_table_create_ptr_string(size_t initial_capacity) {
    return hash_table_create(
        sizeof(void*),
        sizeof(char*),
        initial_capacity,
        hash_pointer_v2,
        compare_pointers,
        copy_pointer,
        free_pointer,
        copy_string,
        free_string
    );
}

HashTable* hash_table_create_ptr_ptr(size_t initial_capacity) {
    return hash_table_create(
        sizeof(void*),
        sizeof(void*),
        initial_capacity,
        hash_pointer_v2,
        compare_pointers,
        copy_pointer,
        free_pointer,
        copy_pointer,
        free_pointer
    );
}







// Hash Table Operations

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


bool hash_table_remove(HashTable* table, const void* key) {
    if (!table || !key) return false;
    
    uint32_t hash = table->hash_func(key, table->node_count);
    size_t index = hash % table->node_count;
    
    HashNode* current = table->nodes[index];
    HashNode* prev = NULL;
    
    while (current) {
        if (current->hash == hash && 
            table->key_compare_func(current->key, key)) {
            
           
            if (prev) {
                prev->next = current->next;
            } else {
                table->nodes[index] = current->next;
            }
            
            
            table->key_free_func(current->key);
            table->value_free_func(current->value);
            free(current);
            
            table->element_count--;
            return true;
        }
        
        prev = current;
        current = current->next;
    }
    
    return false;  
}

bool hash_table_contains(const HashTable* table, const void* key) {
    if (!table || !key) return false;
    
    uint32_t hash = table->hash_func(key, table->node_count);
    size_t index = hash % table->node_count;
    
    HashNode* current = table->nodes[index];
    
    while (current) {
        if (current->hash == hash && 
            table->key_compare_func(current->key, key)) {
            return true;
        }
        current = current->next;
    }
    
    return false;
}


void hash_table_destroy(HashTable* table) {
    if (!table) return;
    
    hash_table_clear(table);
    
    free(table->nodes);
    
    free(table);
}

void hash_table_clear(HashTable* table) {
    if (!table) return;
    
    for (size_t i = 0; i < table->node_count; i++) {
        HashNode* current = table->nodes[i];
        while (current) {
            HashNode* next = current->next;
            
            table->key_free_func(current->key);
            table->value_free_func(current->value);
            free(current);
            
            current = next;
        }
        table->nodes[i] = NULL;
    }
    
    table->element_count = 0;
}


// Hash Table Stats
size_t hash_table_size(const HashTable* table) {
    return table ? table->element_count : 0;
}

size_t hash_table_capacity(const HashTable* table) {
    return table ? table->node_count : 0;
}

bool hash_table_is_empty(const HashTable* table) {
    return !table || table->element_count == 0;
}

double hash_table_load_factor(const HashTable* table) {
    if (!table || table->node_count == 0) return 0.0;
    return (double)table->element_count / (double)table->node_count;
}


// ==================== Iterators ====================

HashTableIterator hash_table_iterator_create(HashTable* table) {
    HashTableIterator iter = {0};
    
    if (table) {
        iter.table = table;
        iter.bucket_index = 0;
        iter.current_node = NULL;
        
        // Find first non-empty bucket
        while (iter.bucket_index < table->node_count && 
               !table->nodes[iter.bucket_index]) {
            iter.bucket_index++;
        }
        
        if (iter.bucket_index < table->node_count) {
            iter.current_node = table->nodes[iter.bucket_index];
        }
    }
    
    return iter;
}

bool hash_table_iterator_has_next(const HashTableIterator* iter) {
    if (!iter || !iter->table) return false;
    
    // If we have a current node, check if it has a next
    if (iter->current_node) {
        // Current node has a next in the same bucket
        if (iter->current_node->next) {
            return true;
        }
        
        // Look for next non-empty bucket
        size_t next_bucket = iter->bucket_index + 1;
        while (next_bucket < iter->table->node_count) {
            if (iter->table->nodes[next_bucket]) {
                return true;
            }
            next_bucket++;
        }
    }
    
    return false;
}

bool hash_table_iterator_next(HashTableIterator* iter, void* key_out, void* value_out) {
    if (!iter || !iter->table || !iter->current_node) {
        return false;
    }
    
    // Copy current key and value to output
    if (key_out) {
        memcpy(key_out, iter->current_node->key, iter->table->key_size);
    }
    if (value_out) {
        memcpy(value_out, iter->current_node->value, iter->table->value_size);
    }
    
    // Move to next node
    if (iter->current_node->next) {
        // Next node in same bucket
        iter->current_node = iter->current_node->next;
    } else {
        // Find next non-empty bucket
        iter->bucket_index++;
        while (iter->bucket_index < iter->table->node_count) {
            if (iter->table->nodes[iter->bucket_index]) {
                iter->current_node = iter->table->nodes[iter->bucket_index];
                return true;
            }
            iter->bucket_index++;
        }
        // No more elements
        iter->current_node = NULL;
    }
    
    return true;
}

void hash_table_iterator_reset(HashTableIterator* iter) {
    if (!iter) return;
    
    iter->bucket_index = 0;
    iter->current_node = NULL;
    
    if (iter->table) {
        // Find first non-empty bucket
        while (iter->bucket_index < iter->table->node_count && 
               !iter->table->nodes[iter->bucket_index]) {
            iter->bucket_index++;
        }
        
        if (iter->bucket_index < iter->table->node_count) {
            iter->current_node = iter->table->nodes[iter->bucket_index];
        }
    }
}

// ==================== Table Operations ====================

bool hash_table_reserve(HashTable* table, size_t new_capacity) {
    if (!table) return false;
    
    if (new_capacity <= table->node_count) {
        return true;  // Already has enough capacity
    }
    
    return hash_table_rehash(table, new_capacity);
}

bool hash_table_rehash(HashTable* table, size_t new_capacity) {
    if (!table) return false;
    
    // Calculate next prime capacity
    size_t actual_capacity = next_prime(new_capacity);
    if (actual_capacity <= table->node_count) {
        actual_capacity = next_prime(table->node_count * 2);
    }
    
    // Create new buckets array
    HashNode** new_buckets = calloc(actual_capacity, sizeof(HashNode*));
    if (!new_buckets) {
        return false;
    }
    
    // Rehash all elements
    for (size_t i = 0; i < table->node_count; i++) {
        HashNode* current = table->nodes[i];
        while (current) {
            HashNode* next = current->next;
            
            // Recalculate hash for new bucket count
            size_t new_index = current->hash % actual_capacity;
            
            // Insert into new bucket
            current->next = new_buckets[new_index];
            new_buckets[new_index] = current;
            
            current = next;
        }
    }
    
    // Update table
    free(table->nodes);
    table->nodes = new_buckets;
    table->node_count = actual_capacity;
    table->resize_threshold = (size_t)(actual_capacity * table->max_load_factor);
    
    return true;
}

HashTable* hash_table_copy(const HashTable* table) {
    if (!table) return NULL;
    
    // Create new table with same configuration
    HashTable* new_table = malloc(sizeof(HashTable));
    if (!new_table) return NULL;
    
    memcpy(new_table, table, sizeof(HashTable));
    
    // Allocate new buckets array
    new_table->nodes = calloc(table->node_count, sizeof(HashNode*));
    if (!new_table->nodes) {
        free(new_table);
        return NULL;
    }
    
    // Copy all nodes
    for (size_t i = 0; i < table->node_count; i++) {
        HashNode* current = table->nodes[i];
        HashNode** new_current_ptr = &new_table->nodes[i];
        
        while (current) {
            // Create new node
            HashNode* new_node = malloc(sizeof(HashNode));
            if (!new_node) {
                // Cleanup on failure
                hash_table_destroy(new_table);
                return NULL;
            }
            
            // Copy node metadata
            new_node->hash = current->hash;
            
            // Copy key using copy function
            new_node->key = table->key_copy_func(current->key);
            if (!new_node->key && table->key_size > 0) {
                free(new_node);
                hash_table_destroy(new_table);
                return NULL;
            }
            
            // Copy value using copy function
            new_node->value = table->value_copy_func(current->value);
            if (!new_node->value && table->value_size > 0) {
                table->key_free_func(new_node->key);
                free(new_node);
                hash_table_destroy(new_table);
                return NULL;
            }
            
            // Link node
            new_node->next = NULL;
            *new_current_ptr = new_node;
            new_current_ptr = &new_node->next;
            
            current = current->next;
        }
    }
    
    return new_table;
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
           table->element_count, table->node_count,
           hash_table_load_factor(table) * 100);
    printf("Key size: %zu bytes, Value size: %zu bytes\n",
           table->key_size, table->value_size);
    printf("Max Load Factor: %.2f\n", table->max_load_factor);
    printf("Resize Threshold: %zu\n", table->resize_threshold);
    printf("\nBucket Contents:\n");
    printf("========================================\n");
    
    for (size_t i = 0; i < table->node_count; i++) {
        HashNode* node = table->nodes[i];
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



void hash_table_print_bucket_distribution(const HashTable* table) {
    if (!table) return;
    
    printf("\n=== HashTable Bucket Distribution ===\n");
    
    // Create a histogram
    size_t max_len = 0;
    for (size_t i = 0; i < table->node_count; i++) {
        size_t len = 0;
        HashNode* node = table->nodes[i];
        while (node) {
            len++;
            node = node->next;
        }
        if (len > max_len) max_len = len;
    }
    
    size_t* histogram = calloc(max_len + 1, sizeof(size_t));
    if (!histogram) return;
    
    for (size_t i = 0; i < table->node_count; i++) {
        size_t len = 0;
        HashNode* node = table->nodes[i];
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
    
    if (!table->nodes) {
        printf("ERROR: Buckets array is NULL\n");
        return false;
    }
    
    if (table->node_count == 0) {
        printf("ERROR: Bucket count is zero\n");
        return false;
    }
    
    if (table->max_load_factor <= 0 || table->max_load_factor > 1.0) {
        printf("ERROR: Invalid max load factor: %.2f\n", table->max_load_factor);
        return false;
    }
    
    // Count elements
    size_t counted_elements = 0;
    for (size_t i = 0; i < table->node_count; i++) {
        HashNode* node = table->nodes[i];
        while (node) {
            counted_elements++;
            
            // Validate hash
            uint32_t computed_hash = table->hash_func(node->key, table->node_count);
            size_t computed_index = computed_hash % table->node_count;
            
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
    fprintf(file, "  Buckets: %zu\n", table->node_count);
    fprintf(file, "  Load Factor: %.2f%%\n", hash_table_load_factor(table) * 100);
    fprintf(file, "  Key Size: %zu bytes\n", table->key_size);
    fprintf(file, "  Value Size: %zu bytes\n", table->value_size);
    fprintf(file, "\n");
    
    fprintf(file, "Contents:\n");
    fprintf(file, "---------\n");
    
    for (size_t i = 0; i < table->node_count; i++) {
        HashNode* node = table->nodes[i];
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
    stats.buckets_memory = table->node_count * sizeof(HashNode*);
    
    // Nodes and their contents
    for (size_t i = 0; i < table->node_count; i++) {
        HashNode* node = table->nodes[i];
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