#ifndef REFACTORPROJECT_STACK_H
#define REFACTORPROJECT_STACK_H

#include "definitions.h"

typedef struct  {
    char key[WORDS];
    int line;
    bool empty;
} Map;

void initElements(Map *m);

void insertElement(Map *m, char *key, int lineNumber);

void checkElement(Map *m, char *key);

void printFooMap(Map *m);

void printVarMap(Map *m);

void sortAndPrintFunctions(Map *m);

void sortAndPrintVariables(Map *m);


void push(struct Node **s, state value);

state pop(struct Node **s);

state peek(struct Node **s);

#endif //REFACTORPROJECT_STACK_H
