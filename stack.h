#ifndef STACK_H
#define STACK_H

// structure for stack
typedef struct Stack {
    void **data;
    int size;
    int capacity;
} Stack;

Stack *createStack();
void pushStack(Stack *stack, void *data);
void *popStack(Stack *stack);
int isEmptyStack(Stack *stack);
void freeStack(Stack *stack);

#endif