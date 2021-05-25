#include "definitions.h"
#include "specialSymbols.h"
#include "wordHandler.h"
#include "newTypes.h"
#include "checkInitialization.h"
#include "unusedTokens.h"
#include "checkRecursion.h"
#include "checkLooping.h"
#include "incorrectWriting.h"
#include <dirent.h>

#define SOURCE_DIRECTORY "Src\\"

#define OUTPUT_DIRECTORY "Out\\"

void swapTexts(char *sourceText, int *sourceSize, char *outputText, int *outputSize) {
    strcpy(sourceText, outputText);
    *sourceSize = *outputSize;
    *outputSize = 0;
}

void readFile(char *sourceText, int *sourceSize, char *directory, char *fileName) {
    char srcDirectory[WORD_LENGTH] = {0};
    strcat(srcDirectory, directory);
    strcat(srcDirectory, fileName);

    FILE *sourceFile = fopen(srcDirectory, "rt");
    *sourceSize = 0;
    while (!feof(sourceFile))
        sourceText[(*sourceSize)++] = (char) getc(sourceFile);
    fclose(sourceFile);
    (*sourceSize)--;
}

void outputFile(const char *outputText, int *outputSize, const char *file) {
    char outDirectory[WORD_LENGTH] = {0};
    strcat(outDirectory, OUTPUT_DIRECTORY);
    strcat(outDirectory, file);
    FILE *outputFile = fopen(outDirectory, "wt");
    for (int j = 0; j < *outputSize; ++j)
        putc(outputText[j], outputFile);
    fclose(outputFile);
    *outputSize = 0;
}

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
    char variables[WORD_LENGTH][NAME_SIZE] = {0 };
    int variablesSize = 0;

    //Все функции
    char functions[WORD_LENGTH][NAME_SIZE] = {0 };
    int functionsSize = 0;

    // Мапы для переменных и функций. Будем помечать, какие были, и использовались ли они
    Map variablesMap[MAP_SIZE], functionsMap[MAP_SIZE];
    initElements(variablesMap);
    initElements(functionsMap);

    // Переменная для определения номера строки в файле
    int lineNumber = 1;

    // Директория файлов
    DIR *dir = opendir(SOURCE_DIRECTORY);

    // Текущий файл
    struct dirent *ent = NULL;
    if (dir == NULL)
        return -1;

    // Массив файлов
    char files[WORD_COUNT][WORD_LENGTH];
    int fileCount = 0;

    // Запоминаем названия всех файлов
    while ((ent = readdir(dir))) {
        if (ent->d_namlen > 2) {
            char directory[WORD_LENGTH] = {0};
            strcat(directory, ent->d_name);
            strcpy(files[fileCount++], directory);
        }
    }
    closedir(dir);

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
        wordHandler(sourceText, sourceSize, outputText, &outputSize, now, nowSize);

        // Выводим файл в out
        outputFile(outputText, &outputSize, files[i]);
    }

    // Соберём все инициализации переменных и функций во всех файлах STEP 2
    for (int i = 0; i < fileCount; ++i) {
        readFile(sourceText, &sourceSize, OUTPUT_DIRECTORY, files[i]);

        // Step 4 - checking for initialization of variables TODO many files
        checkInit(sourceText, sourceSize, now, nowSize, variablesMap, functionsMap, &lineNumber, files[i]);
    }

    // Дособираем инициализации переменных и проверяем на использование
    for (int i = 0; i < fileCount; ++i) {
        readFile(sourceText, &sourceSize, OUTPUT_DIRECTORY, files[i]);

        // Step 5 - searching for unused variables and functions TODO many files
        checkUnused(sourceText, sourceSize, now, nowSize, variablesMap, functionsMap, &lineNumber, files[i]);
    }

    // Проанализируем каждый файл STEP 3
    for (int i = 0; i < fileCount; ++i) {
        printf("----------------------------\nFile '%s':\n----------------------------\n", files[i]);

        readFile(sourceText, &sourceSize, OUTPUT_DIRECTORY, files[i]);

        // Step 6 - checking for endless loops TODO many files
        checkLooping(sourceText, sourceSize, variables, &variablesSize,
                     functions, &functionsSize);
        // Step 7
        incorrectWriting(now, &nowSize, initialSize, sourceText, sourceSize, variables, &variablesSize,
                         functions, &functionsSize);

        // Выведем unused
        printFooMap(functionsMap, files[i]);
        printVarMap(variablesMap, files[i]);
    }



    //TODO!!!!!!!!!!!! checkRecursion

    return 0;

    /*for (int i = 0; i < fileCount; ++i) {

        char srcDirectory[WORD_LENGTH] = {0};
        strcat(srcDirectory, SOURCE_DIRECTORY);
        strcat(srcDirectory, files[i]);
        readFile(srcDirectory, sourceText, &sourceSize);

        *//*
         * 1) Собрать типы данных
         * 2) Отформатировать текст (Step 1 - 3.1)
         * Бегаем по отформатированным файлам
         * 3) checkInit по всем файлам (собрать все инициализации)
         * 4) Все оставшиеся функции по каждому файлу
         * Примечание: printMaps должны быть после всего анализа, чтобы вывести конечный результат
         *//*

        // Step 1 - special symbols
        processSpecialSymbols(sourceText, sourceSize, outputText, &outputSize);

        // Step 1.1 - output <-> input
        swapTexts(sourceText, &sourceSize, outputText, &outputSize);

        // Step 2
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
        lineNumber = 1;

        // Step 6 - checking for endless loops TODO many files
        checkLooping(sourceText, sourceSize, variables, &variablesSize,
                     functions, &functionsSize);
        // Step 7
        incorrectWriting(now, &nowSize, initialSize, sourceText, sourceSize, variables, &variablesSize,
                         functions, &functionsSize);

        // Output new code
        *//*char outDirectory[WORD_LENGTH] = {0};
        strcat(outDirectory, OUTPUT_DIRECTORY);
        strcat(outDirectory, files[i]);
        outputFile(outputText, &outputSize, outDirectory);*//*

        //printf("\n");
    }*/


    // Formatting
    // Step 1 - special symbols
    /*processSpecialSymbols(sourceText, sourceSize, outputText, &outputSize);

    // Step 1.1 - output <-> input
    swapTexts(sourceText, &sourceSize, outputText, &outputSize);

    // Step 2 - custom data types and checking for the correctness of variables and functions TODO many files
    newTypes(now, &nowSize, initialSize, sourceText, sourceSize);

    // Step 3 - formatting
    wordHandler(sourceText, sourceSize, outputText, &outputSize, now, nowSize);

    // Step 3.1 - exchange texts and finalize size of output text (outputSize = sourceSize)
    swapTexts(sourceText, &sourceSize, outputText, &outputSize);
    outputSize = sourceSize;

    incorrectWriting(now, &nowSize, initialSize, sourceText, sourceSize, variables, &variablesSize,
                     functions, &functionsSize);

    // Step 4 - checking for initialization of variables TODO many files
    checkInit(sourceText, sourceSize, now, nowSize, variablesMap, functionsMap, &lineNumber);

    // Step 5 - searching for unused variables and functions TODO many files
    checkUnused(sourceText, sourceSize, now, nowSize, variablesMap, functionsMap, &lineNumber);

    // Step 6 - checking for endless loops TODO many files
    checkLooping(sourceText, sourceSize, variables, &variablesSize,
                 functions, &functionsSize);

    // Step 7 - revealing recursion chains
    checkRecursion(sourceText, sourceSize, now, nowSize); // TODO file-argument*/

    // Formatting final - output new code
    /*FILE *outputFile = fopen("output.c", "wt");
    for (int i = 0; i < outputSize; ++i)
        putc(outputText[i], outputFile);
    fclose(outputFile);*/
}