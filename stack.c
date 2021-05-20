#include "stack.h"
#include <malloc.h>

void push(struct Node **s, state value) {
    struct Node *temp = (struct Node *) malloc(sizeof(struct Node));
    temp->value = value;
    temp->next = *s;
    *s = temp;
}

state pop(struct Node **s) {
    state value = (*s)->value;
    struct Node *next = (*s)->next;
    free(*s);
    *s = next;
    return value;
}

state peek(struct Node **s) {
    if ((*s))
        return (*s)->value;
    else
        return EMPTY;
}