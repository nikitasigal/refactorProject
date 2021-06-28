#ifndef REFACTORPROJECT_WORDHANDLER_H
#define REFACTORPROJECT_WORDHANDLER_H

#include "definitions.h"
#include "structs.h"
#include "generalFunctions.h"

void
wordHandler(char *input, int inputSize, char *output, int *outputSize, stateTypes *now, int nowSize, int *nestingArray,
            int *nestingSize);

#endif //REFACTORPROJECT_WORDHANDLER_H
