#ifndef REFACTORPROJECT_NEWTYPES_H
#define REFACTORPROJECT_NEWTYPES_H

#include "definitions.h"
#include "structs.h"
#include "generalFunctions.h"

void newTypes(stateTypes *now, int *nowSize, int initialSize, char *input, int inputSize, char variables[][NAME_SIZE], int *variablesSize,
                                                                                          char functions[][NAME_SIZE], int *functionsSize);

#endif //REFACTORPROJECT_NEWTYPES_H
