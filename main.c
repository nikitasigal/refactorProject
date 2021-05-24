#include "definitions.h"
#include "specialSymbols.h"
#include "wordHandler.h"
#include "newTypes.h"
#include "checkInitialization.h"
#include "unusedTokens.h"
#include "checkRecursion.h"

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

    // Мапы для переменных и функций. Будем помечать, какие были, и использовались ли они
    Map variablesMap[MAP_SIZE], functionsMap[MAP_SIZE];
    initElements(variablesMap);
    initElements(functionsMap);


    //
    /*struct TreeNode *tree = NULL;
    tree = addNode(NULL, tree, NULL, "a", NULL);
    addNode(tree, tree, NULL, "b", "a");
    addNode(tree, tree, NULL, "c", "a");
    addNode(tree, tree, NULL, "d", "c");
    addNode(tree, tree, NULL, "d", "b");

    addNode(tree, tree, NULL, "a", "d");*/
    //addNode("a", "b", tree, NULL);

    //

    // Переменная для определения номера строки в файле
    int lineNumber = 1;

    // Formatting
    // Step 1 - special symbols
    processSpecialSymbols(sourceText, sourceSize, outputText, &outputSize);

    // Step 1.1 - output <-> input
    swapTexts(sourceText, &sourceSize, outputText, &outputSize);

    // Step 2 - custom data types and checking for the correctness of variables and functions TODO many files
    newTypes(now, &nowSize, initialSize, sourceText, sourceSize);

    // Step 3 - formatting
    wordHandler(sourceText, sourceSize, outputText, &outputSize, now, nowSize);

    // Step 3.1 - exchange texts and finalize size of output text (outputSize = sourceSize)
    swapTexts(sourceText, &sourceSize, outputText, &outputSize);
    outputSize = sourceSize;

    // Step 4 - checking for initialization of variables TODO many files
    checkInit(sourceText, sourceSize, now, nowSize, variablesMap, functionsMap, &lineNumber);

    // Step 5 - searching for unused variables and functions TODO many files
    checkUnused(sourceText, sourceSize, now, nowSize, variablesMap, functionsMap, &lineNumber);

    checkRecursion(sourceText, sourceSize, now, nowSize); // TODO file-argument

    // Formatting final - output new code
    FILE *outputFile = fopen("output.c", "wt");
    for (int i = 0; i < outputSize; ++i)
        putc(outputText[i], outputFile);
    fclose(outputFile);
}