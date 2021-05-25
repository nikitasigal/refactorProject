#ifndef REFACTORPROJECT_INCORRECTWRITING_H
#define REFACTORPROJECT_INCORRECTWRITING_H

#include "definitions.h"
#include "structs.h"
#include "generalFunctions.h"

void incorrectWriting(stateTypes *now, int *nowSize, int initialSize, char *input, int inputSize, char variables[][NAME_SIZE], int *variablesSize,
              char functions[][NAME_SIZE], int *functionsSize);

#endif //REFACTORPROJECT_INCORRECTWRITING_H
