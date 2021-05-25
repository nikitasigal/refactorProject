#ifndef REFACTORPROJECT_FILEHANDLING_H
#define REFACTORPROJECT_FILEHANDLING_H

#include "definitions.h"
#include <dirent.h>

void swapTexts(char *sourceText, int *sourceSize, char *outputText, int *outputSize);

void readFile(char *sourceText, int *sourceSize, char *directory, char *fileName);

void outputFile(const char *outputText, int *outputSize, const char *file);

void readFileNames(char files[][WORD_LENGTH], int *fileCount);

#endif //REFACTORPROJECT_FILEHANDLING_H
