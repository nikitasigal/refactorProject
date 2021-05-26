#ifndef REFACTORPROJECT_NEWTYPES_H
#define REFACTORPROJECT_NEWTYPES_H

#include "definitions.h"
#include "structs.h"
#include "generalFunctions.h"

void newTypes(stateTypes *now, int *nowSize, char *input, int inputSize);

void skipSpecialWords(const char *input, int inputSize, int *j, stateTypes *now, int nowSize);

void skip2(const char *input, int *i, int inputSize, int *lineNumber);


#endif //REFACTORPROJECT_NEWTYPES_H
