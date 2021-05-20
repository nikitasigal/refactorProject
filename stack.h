#ifndef REFACTORPROJECT_STACK_H
#define REFACTORPROJECT_STACK_H

#include "definitions.h"

void push(struct Node **s, state value);

state pop(struct Node **s);

state peek(struct Node **s);

#endif //REFACTORPROJECT_STACK_H
