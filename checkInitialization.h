#ifndef REFACTORPROJECT_CHECKINITIALIZATION_H
#define REFACTORPROJECT_CHECKINITIALIZATION_H

#define NUMBER_OF_VARIABLES 256

#include "definitions.h"
#include "structs.h"
#include "generalFunctions.h"

void checkInit(char *input, int inputSize, stateTypes *now, int nowSize, Map *variablesMap, Map *functionsMap, int *lineNumber);

void skip3(const char *input, int *i, int inputSize, int *lineNumber);

void skipComments(const char *input, int inputSize, int *i, int *lineNumber);

void processTypedef(const char *input, int *i, int inputSize, int *lineNumber);

bool checkStruct(const char *input, int *i, int inputSize, int *lineNumber);

void skipTypes(char *input, int *i, int inputSize, stateTypes *now, int nowSize, int *lineNumber);

#endif //REFACTORPROJECT_CHECKINITIALIZATION_H
