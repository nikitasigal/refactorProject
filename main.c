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

    // Переменная для определения номера строки в файле
    int lineNumber = 1;

    // Массив файлов
    char files[WORD_COUNT][WORD_LENGTH];
    int fileCount = 0;

    // Запоминаем названия всех файлов
    readFileNames(files, &fileCount);

    // Хранилище nesting'ов
    int nestingArray[WORD_COUNT];
    int nestingSize = 0;

    // TODO исправить комментарии
    // Форматируем все файлы и собираем новые типы данных STEP 1
    for (int i = 0; i < fileCount; ++i) {
        readFile(sourceText, &sourceSize, SOURCE_DIRECTORY, files[i]);

        // Step 1 - special symbols
        processSpecialSymbols(sourceText, sourceSize, outputText, &outputSize);

        // Step 1.1 - output <-> input
        swapTexts(sourceText, &sourceSize, outputText, &outputSize);

        // Step 2
        newTypes(now, &nowSize, initialSize, sourceText, sourceSize);

        // Step 3 - formatting
        wordHandler(sourceText, sourceSize, outputText, &outputSize, now, nowSize, nestingArray, &nestingSize);

        // Выводим файл в out
        outputFile(outputText, &outputSize, files[i]);
    }

    // Соберём все инициализации переменных и функций во всех файлах STEP 2
    for (int i = 0; i < fileCount; ++i) {
        readFile(sourceText, &sourceSize, OUTPUT_DIRECTORY, files[i]);

        // Step 4 - checking for initialization of variables
        checkInit(sourceText, sourceSize, now, nowSize, variablesMap, functionsMap, variablesInitMap, &lineNumber,
                  files[i]);
    }

    // Дособираем инициализации переменных для мапа неиспользованных переменных и проверяем на использование переменные
    // и функции STEP 3
    for (int i = 0; i < fileCount; ++i) {
        readFile(sourceText, &sourceSize, OUTPUT_DIRECTORY, files[i]);

        // Step 5 - searching for unused variables and functions
        checkUnused(sourceText, sourceSize, now, nowSize, variablesMap, functionsMap, &lineNumber, files[i]);
    }

    // Проанализируем каждый файл STEP 4
    for (int i = 0; i < fileCount; ++i) {
        printf("----------------------------\nFile '%s':\n----------------------------\n", files[i]);

        readFile(sourceText, &sourceSize, OUTPUT_DIRECTORY, files[i]);

        printf("\nMaximum nesting of loops: %d\n", nestingArray[i]);

        //Step 6 - checking variables/functions/data types if their names are wrong
        incorrectWriting(now, &nowSize, initialSize, sourceText, sourceSize, variables, &variablesSize,
                        functions, &functionsSize);

        // Step 7 - checking for endless loops
        checkLooping(sourceText, sourceSize, variables, &variablesSize,
                     functions, &functionsSize);

        // Выведем не инициализированные переменные и неиспользованные переменные и функции
        printVarInitMap(variablesInitMap, files[i]);
        printFooMap(functionsMap, files[i]);
        printVarMap(variablesMap, files[i]);
    }

    // Выводим рекурсивные цепочки STEP 5
    checkRecursion(now, nowSize, files, fileCount);

    return 0;
}