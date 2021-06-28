#ifndef REFACTORPROJECT_GENERALFUNCTIONS_H
#define REFACTORPROJECT_GENERALFUNCTIONS_H

#include "definitions.h"

// Очистка слова
void clearWord(char *word, int *wordSize);

void readWord(const char *input, char *word, int *wordSize, int *i);

void universalSkip(const char *input, int *i, int inputSize, int *lineNumber);

void skipComments(const char *input, int inputSize, int *i, int *lineNumber);

#endif //REFACTORPROJECT_GENERALFUNCTIONS_H
