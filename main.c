#include "definitions.h"
#include "specialSymbols.h"
#include "wordHandler.h"
#include "newTypes.h"
#include "checkInitialization.h"
#include "unusedTokens.h"
#include "checkRecursion.h"
#include "checkLooping.h"
#include "incorrectWriting.h"
#include "fileHandling.h"

int main() {
    // Текст из файла
    char sourceText[TEXT_SIZE] = {0};
    int sourceSize = 0;

    // Отформатированный текст
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

    //Все переменные
    char variables[WORD_LENGTH][NAME_SIZE] = {0};
    int variablesSize = 0;

    //Все функции
    char functions[WORD_LENGTH][NAME_SIZE] = {0};
    int functionsSize = 0;

    // Мапы для переменных и функций. Будем помечать, какие были, и использовались ли они
    Map variablesMap[MAP_SIZE], functionsMap[MAP_SIZE], variablesInitMap[MAP_SIZE];
    initElements(variablesMap);
    initElements(functionsMap);
    initElements(variablesInitMap);

    // Хранилище вложенности циклов
    int nestingArray[WORD_COUNT];
    int nestingSize = 0;

    // Переменная для определения номера строки в файле
    int lineNumber = 1;

    // Массив файлов
    char files[WORD_COUNT][WORD_LENGTH];
    int fileCount = 0;

    // Запоминаем названия всех файлов
    readFileNames(files, &fileCount);

    // At the beginning, collect all new data types
    for (int i = 0; i < fileCount; ++i) {
        readFile(sourceText, &sourceSize, SOURCE_DIRECTORY, files[i]);

        // Step 1 - memorizing new data types
        newTypes(now, &nowSize, sourceText, sourceSize);
    }

    // Format all the files and collect new data types
    for (int i = 0; i < fileCount; ++i) {
        readFile(sourceText, &sourceSize, SOURCE_DIRECTORY, files[i]);

        // Step 2 - special symbols
        processSpecialSymbols(sourceText, sourceSize, outputText, &outputSize);

        // Step 2.1 - output <-> input
        swapTexts(sourceText, &sourceSize, outputText, &outputSize);

        // Step 3 - formatting and checking for nesting loops
        wordHandler(sourceText, sourceSize, outputText, &outputSize, now, nowSize, nestingArray, &nestingSize);

        // Выводим файл в out
        outputFile(outputText, &outputSize, files[i]);
    }

    // Collect almost all variable and function initializations in every file
    for (int i = 0; i < fileCount; ++i) {
        readFile(sourceText, &sourceSize, OUTPUT_DIRECTORY, files[i]);

        // Step 4 - checking for initialization of variables
        checkInit(sourceText, sourceSize, now, nowSize, variablesMap, functionsMap, variablesInitMap, &lineNumber,
                  files[i]);
    }

    // Finish collecting the remaining variables for map of unused variables and check for use of variables and functions
    for (int i = 0; i < fileCount; ++i) {
        readFile(sourceText, &sourceSize, OUTPUT_DIRECTORY, files[i]);

        // Step 5 - searching for unused variables and functions
        checkUnused(sourceText, sourceSize, now, nowSize, variablesMap, functionsMap, &lineNumber, files[i]);
    }

    // Analyse every file
    for (int i = 0; i < fileCount; ++i) {
        printf("----------------------------\nFile '%s':\n----------------------------\n", files[i]);

        readFile(sourceText, &sourceSize, OUTPUT_DIRECTORY, files[i]);

        // Step 6 - output max nesting of loops
        printf("\nMaximum nesting of loops: %d\n", nestingArray[i]);

        // Step 7 - checking variables/functions/data types if their names are wrong
        incorrectWriting(now, &nowSize, sourceText, sourceSize, variables, &variablesSize,
                         functions, &functionsSize);

        // Step 8 - checking for endless loops
        checkLooping(sourceText, sourceSize, variables, &variablesSize);

        // Output not initialized variables and unused variables and functions
        printVarInitMap(variablesInitMap, files[i]);
        printFooMap(functionsMap, files[i]);
        printVarMap(variablesMap, files[i]);
    }

    // Step 9-10 - revealing recursion chains and output tree of callings
    checkRecursion(now, nowSize, files, fileCount);

    return 0;
}