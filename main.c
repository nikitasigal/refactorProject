#include "definitions.h"
#include "specialSymbols.h"
#include "wordHandler.h"
#include "newTypes.h"
#include "checkInitialization.h"
#include "unusedTokens.h"
#include "checkLooping.h"

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
    int initialSize = nowSize;
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

    //Все переменные
    char variables[WORDS][NAME_SIZE] = { 0 };
    int variablesSize = 0;

    //Все функции
    char functions[WORDS][NAME_SIZE] = { 0 };
    int functionsSize = 0;

    // Мапы для переменных и функций. Будем помечать, какие были, и использовались ли они
    Map variablesMap[MAP_SIZE], functionsMap[MAP_SIZE];
    initElements(variablesMap);
    initElements(functionsMap);

    // Переменная для определения номера строки в файле
    int lineNumber = 1;

    // Formatting
    // Step 1 - special symbols
    processSpecialSymbols(sourceText, sourceSize, outputText, &outputSize);

    // Step 1.1 - output <-> input
    swapTexts(sourceText, &sourceSize, outputText, &outputSize);

    // Step 2 - custom data types and checking for the correctness of variables and functions TODO many files
    newTypes(now, &nowSize, initialSize, sourceText, sourceSize, variables, &variablesSize,
                                                                 functions, &functionsSize);

    // Step 3 - formatting
    wordHandler(sourceText, sourceSize, outputText, &outputSize, now, nowSize);

    // Step 3.1 - exchange texts and finalize size of output text (outputSize = sourceSize)
    swapTexts(sourceText, &sourceSize, outputText, &outputSize);
    outputSize = sourceSize;

    // Step 4 - checking for initialization of variables TODO many files
    checkInit(sourceText, sourceSize, now, nowSize, variablesMap, functionsMap, &lineNumber);

    // Step 5 - searching for unused variables and functions
    checkUnused(sourceText, sourceSize, now, nowSize, variablesMap, functionsMap, &lineNumber);

    checkLooping(sourceText, sourceSize, variables, &variablesSize,
                                         functions, &functionsSize);

    // Formatting final - output new code
    FILE *outputFile = fopen("output.c", "wt");
    for (int i = 0; i < outputSize; ++i)
        putc(outputText[i], outputFile);
    fclose(outputFile);
}