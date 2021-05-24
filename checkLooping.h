#ifndef REFACTORPROJECT_CHECKLOOPING_H
#define REFACTORPROJECT_CHECKLOOPING_H

#include "definitions.h"
#include "structs.h"
#include "generalFunctions.h"

void checkLooping (char *input, int inputSize, char variables[][NAME_SIZE], int *variablesSize,
                                               char functions[][NAME_SIZE], int *functionsSize);

#endif //REFACTORPROJECT_CHECKLOOPING_H
