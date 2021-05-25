#ifndef REFACTORPROJECT_UNUSEDTOKENS_H
#define REFACTORPROJECT_UNUSEDTOKENS_H

#include "definitions.h"
#include "structs.h"
#include "checkInitialization.h"
#include "generalFunctions.h"

void checkUnused(char *input, int inputSize, stateTypes *now, int nowSize, Map *variablesMap, Map *functionsMap,
                 int *lineNumber, char *file);

#endif //REFACTORPROJECT_UNUSEDTOKENS_H
