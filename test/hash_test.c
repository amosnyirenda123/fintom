#include <stdlib.h>
#include <stdio.h>
#include "../include/hash/hash_table.h"
#include "../include/common.h"


void example_string_int() {
    printf("=== String -> Int Map Example ===\n");
    
    HashTable* table = hash_table_create_string_int(8);
    
    // Insert string->int pairs
    char* keys[] = {"apples", "bananas", "cherries"};
    int values[] = {5, 10, 15};
    
    for (int i = 0; i < 3; i++) {
        hash_table_insert(table, &keys[i], &values[i]);
    }
    
    // Retrieve values
    char* key = "bananas";
    int* result = hash_table_get(table, &key);
    if (result) {
        printf("Found '%s': %d\n", key, *result);
    }
    
    hash_table_destroy(table);
}

void example_int_string() {
    printf("\n=== Int -> String Map Example ===\n");
    
    HashTable* table = hash_table_create_int_string(8);
    
    // Insert int->string pairs
    int keys[] = {100, 200, 300};
    char* values[] = {"error", "warning", "info"};
    
    for (int i = 0; i < 3; i++) {
        hash_table_insert(table, &keys[i], &values[i]);
    }
    
    // Retrieve values
    int key = 200;
    char** result = hash_table_get(table, &key);
    if (result) {
        printf("Found %d: '%s'\n", key, *result);
    }
    
    hash_table_destroy(table);
}

void example_int_int() {
    printf("\n=== Int -> Int Map Example ===\n");
    
    HashTable* table = hash_table_create_int_int(8);
    
    // Insert int->int pairs (e.g., ID -> age)
    int keys[] = {1, 2, 3};
    int values[] = {25, 30, 35};
    
    for (int i = 0; i < 3; i++) {
        hash_table_insert(table, &keys[i], &values[i]);
    }
    
    // Update a value
    int key = 2;
    int new_age = 31;
    hash_table_insert(table, &key, &new_age);  // Will update existing
    
    // Check all entries
    HashTableIterator iter = hash_table_iterator_create(table);
    int k, v;
    while (hash_table_iterator_next(&iter, &k, &v)) {
        printf("ID %d -> Age %d\n", k, v);
    }
    
    hash_table_destroy(table);
}

// Example with custom integer keys
void example_custom_ints() {
    printf("\n=== Custom Integer Usage ===\n");
    
    // Using the int comparison and copy functions directly
    int a = 42;
    int b = 42;
    int c = 100;
    
    printf("compare_ints(&a, &b): %s\n", 
           compare_ints(&a, &b) ? "true" : "false");  // true
    
    printf("compare_ints(&a, &c): %s\n", 
           compare_ints(&a, &c) ? "true" : "false");  // false
    
    // Test copy function
    int* copy = copy_int(&a);
    printf("Copied value: %d\n", *copy);  // 42
    free_int(copy);
}


int main()
{

    //example_string_int();
    // example_int_string();
    example_int_int();
    example_custom_ints();

    
    #ifdef HASH_TABLE_DEBUG
        

    #endif

    return EXIT_SUCCESS;
}