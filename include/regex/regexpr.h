#ifndef REGEXPR_H
#define REGEXPR_H

#define MAX_LEN 100

typedef enum {
    TOKEN_LITERAL,
    TOKEN_UNION,       // '|'
    TOKEN_STAR,        // '*'
    TOKEN_PLUS,        // '+'
    TOKEN_QUESTION,    // '?'
    TOKEN_CONCAT,      // Explicit concatenation
    TOKEN_LPAREN,      // '('
    TOKEN_RPAREN,      // ')'
    TOKEN_END
} TokenType;

typedef struct {
    char data[MAX_LEN];
    int top;
} Stack;


int precedence(const char op);
void push(Stack *s, const char c);
char pop(Stack *s);
char peek(const Stack *s);
int is_stack_empty(const Stack *s);
char* infix_to_postfix(const char *regex);
int needs_concat(const char prev, const char curr);



#endif //REGEXPR_H