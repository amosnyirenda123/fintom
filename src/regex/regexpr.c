#include "../../include/regex/regexpr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>



int precedence(const char op) {
    switch (op) {
        case '*': case '+': case '?': return 3;
        case '.': return 2;
        case '|': return 1;
        default: return 0;
    }
}


void push(Stack *s, const char c) { s->data[++s->top] = c; }
char pop(Stack *s) { return (s->top == -1) ? '\0' : s->data[s->top--]; }
char peek(const Stack *s) { return (s->top == -1) ? '\0' : s->data[s->top]; }
int is_stack_empty(const Stack *s) { return s->top == -1; }


int needs_concat(const char prev, const char curr) {
    if (prev == '\0') return 0;
    // Insert concatenation if:
    // - prev is a letter or `)` and curr is a letter or `(` or `*`/`+`/`?`
    if ((isalnum(prev) || prev == ')' || prev == '*' || prev == '+' || prev == '?') && (isalnum(curr) || curr == '(')){
        return 1;
    }
    return 0;
}

char* infix_to_postfix(const char *regex) {
    Stack opStack;
    opStack.top = -1;

    char *postfix = malloc(MAX_LEN * sizeof(char));

    int j = 0;
    char prev = '\0';

    for (int i = 0; regex[i] != '\0'; i++) {
        const char c = regex[i];

        // Insert explicit concatenation `·`
        if (needs_concat(prev, c)) {
            while (!is_stack_empty(&opStack) && precedence(peek(&opStack)) >= precedence('.'))
                postfix[j++] = pop(&opStack);
            push(&opStack, '.');
        }

        if (isalnum(c)) {
            postfix[j++] = c;
        } else if (c == '(') {
            push(&opStack, c);
        } else if (c == ')') {
            while (!is_stack_empty(&opStack) && peek(&opStack) != '(')
                postfix[j++] = pop(&opStack);
            pop(&opStack);
        } else {
            while (!is_stack_empty(&opStack) && precedence(peek(&opStack)) >= precedence(c))
                postfix[j++] = pop(&opStack);
            push(&opStack, c);
        }

        prev = c;
    }

    // remaining operators
    while (!is_stack_empty(&opStack))
        postfix[j++] = pop(&opStack);

    postfix[j] = '\0';

    return postfix;
}