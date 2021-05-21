#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "specialSymbols.h"
#include "wordHandler.h"
#include "newTypes.h"
#include "checkInitialization.h"

void swapTexts(char *sourceText, int *sourceSize, char *outputText, int *outputSize) {
    strcpy(sourceText, outputText);
    *sourceSize = *outputSize;
    *outputSize = 0;
}

int main() {
    FILE *sourceFile = fopen("input.c", "rt");
    char sourceText[TEXT_SIZE] = {0};
    int sourceSize = 0;
    while (!feof(sourceFile))
        sourceText[sourceSize++] = (char) getc(sourceFile);
    fclose(sourceFile);
    sourceSize--;

    char outputText[TEXT_SIZE] = {0};
    int outputSize = 0;

    //now - Массив состояний + их отдельного слова. nowSize - изначальный размер массива (он увеличится, если будут новые типы данных)
    int nowSize = 19;
    stateTypes now[WORDS_FOR_STATE_NUM] = {{"while",   FOR},
                                           {"for",     FOR},
                                           {"switch",  IF},
                                           {"if",      IF},
                                           {"int",     INIT},
                                           {"char",    INIT},
                                           {"double",  INIT},
                                           {"float",   INIT},
                                           {"short",   INIT},
                                           {"long",    INIT},
                                           {"bool",    INIT},
                                           {"void",    INIT},
                                           {"typedef", TYPEDEF},
                                           {"struct",  STRUCT},
                                           {"enum",    STRUCT},
                                           {"case",    CASE},
                                           {"default", CASE},
                                           {"else",    ELSE}};

    // Formatting
    // Step 1 - special symbols
    processSpecialSymbols(sourceText, sourceSize, outputText, &outputSize);

    swapTexts(sourceText, &sourceSize, outputText, &outputSize);

    // Step 2 - custom data types and checking for the correctness of variables and functions
    newTypes(now, &nowSize, sourceText, sourceSize);

    // Step 3 - formatting
    wordHandler(sourceText, sourceSize, outputText, &outputSize, now, nowSize);

    // Step 4 - checking for initialization of variables
    checkInit(sourceText, sourceSize, now, nowSize);

    // Formatting final - output new code
    FILE *outputFile = fopen("output.c", "wt");
    for (int i = 0; i < outputSize; ++i)
        putc(outputText[i], outputFile);
    fclose(outputFile);
}