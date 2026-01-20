#include "../include/fa/fa.h"



#define FA_STACK_DEFAULT_CAPACITY 16

fa_stack* fa_stack_create(int initial_capacity) {
    fa_stack *stack = malloc(sizeof(fa_stack));
    if (!stack) return NULL;
    
    if (!fa_stack_init(stack, initial_capacity)) {
        free(stack);
        return NULL;
    }
    return stack;
}

bool fa_stack_init(fa_stack *stack, int initial_capacity) {
    if (initial_capacity <= 0) {
        initial_capacity = FA_STACK_DEFAULT_CAPACITY;
    }
    
    stack->items = malloc(sizeof(fa_auto*) * initial_capacity);
    if (!stack->items) return false;
    
    stack->top = -1;
    stack->capacity = initial_capacity;
    return true;
}

bool fa_stack_push(fa_stack *stack, fa_auto *automaton) {
    if (stack->top + 1 >= stack->capacity) {
        // Double capacity
        int new_capacity = stack->capacity * 2;
        fa_auto **new_items = realloc(stack->items, 
                                      sizeof(fa_auto*) * new_capacity);
        if (!new_items) return false;
        
        stack->items = new_items;
        stack->capacity = new_capacity;
    }
    
    stack->items[++stack->top] = automaton;
    return true;
}