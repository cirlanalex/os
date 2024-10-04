#include <stdio.h>
#include <stdlib.h>

#include "stack.h"

// create a new stack
Stack *createStack() {
    Stack *stack = malloc(sizeof(Stack));
    stack->size = 0;
    stack->capacity = 1;
    stack->data = malloc(sizeof(void *));
    return stack;
}

// push an element to the stack
void pushStack(Stack *stack, void *element) {
    if (stack->size == stack->capacity) {
        stack->capacity *= 2;
        stack->data = realloc(stack->data, stack->capacity * sizeof(void *));
    }
    stack->data[stack->size] = element;
    stack->size++;
}

// pop an element from the stack
void *popStack(Stack *stack) {
    if (isEmptyStack(stack)) {
        return NULL;
    }
    stack->size--;
    return stack->data[stack->size];
}

// check if the stack is empty
int isEmptyStack(Stack *stack) {
    return stack->size == 0;
}

// free the stack
void freeStack(Stack *stack) {
    for (int i = 0; i < stack->size; i++) {
        free(stack->data[i]);
    }
    free(stack->data);
    free(stack);
}