#include <stdlib.h>
#include <stdio.h>

#include "include/set/set.h"

int main()
{
    Set* int_set = set_create(sizeof(int), compare_int, hash_int);
    for (int i = 0; i < 10; i++) {
        set_insert(int_set, &i);
    }

    Set* str_set = set_create(sizeof(char*), compare_string, hash_string);
    const char* words[] = {"apple", "banana", "cherry", NULL};
    for (int i = 0; words[i]; i++) {
        set_insert(str_set, &words[i]);
    }

    Set* setA = set_create(sizeof(int), compare_int, hash_int);
    Set* setB = set_create(sizeof(int), compare_int, hash_int);

    int a_values[] = {1, 2, 3, 4};
    int b_values[] = {3, 4, 5, 6};
    
    for (int i = 0; i < 4; i++) {
        set_insert(setA, &a_values[i]);
        set_insert(setB, &b_values[i]);
    }

    Set* union_set = set_union(setA, setB);
    Set* inter_set = set_intersection(setA, setB);
    Set* diff_set = set_difference(setA, setB);


    
    #ifdef SET_DEBUG_ENABLE
        printf("Debug enabled! Printing set:\n");
        set_print(str_set, print_string);
        set_print(int_set, print_int);
        set_print(setA, print_int);
        set_print(setB, print_int);
        set_print(union_set, print_int);
        set_print(inter_set, print_int);
        set_print(diff_set, print_int);

    #endif

    set_destroy(int_set);
    set_destroy(str_set);
    set_destroy(setA);
    set_destroy(setB);
    set_destroy(union_set);
    set_destroy(inter_set);
    set_destroy(diff_set);


    printf("Hello world!");
    return EXIT_SUCCESS;
}