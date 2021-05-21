#include "checkInitialization.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/*
 * Структура для переменной. Переменная имеет имя и состояние "инициализирована" / "неинициализирована"
 */
typedef struct {
    char value[WORDS];
    bool isInitialized;
} VARIABLE;

// Очистка слова
void clearWord2(char *word, int *wordSize) {
    for (int l = 0; l < *wordSize; l++)
        word[l] = 0;
    (*wordSize) = 0;
}

// skip версии 3.0. Скипает ненужные знаки, не печатая их, как делает это skip 1.0
void skip3(const char *input, int *i, int inputSize) {
    while (*i < inputSize) {
        // Пропускаем пробелы, \n, \t
        if (input[*i] == ' ' || input[*i] == '\n' || input[*i] == '\t' || input[*i] == ',' || input[*i] == '*' ||
            input[*i] == '&') { // TODO если будут баги, то возможно из-за ( )

            (*i)++;
            continue;
        }

        // Пропускаем кавычки
        if (input[*i] == '\'') {
            (*i)++;
            while ((input[*i - 1] == '\\' && input[*i - 2] != '\\') || input[*i] != '\'')
                (*i)++;
            (*i)++;
            continue;
        }
        if (input[*i] == '"') {
            (*i)++;
            while (input[*i - 1] == '\\' || input[*i] != '"')
                (*i)++;
            (*i)++;
            continue;
        }

        // Пропускаем комментарии
        if (input[*i] == '/') {
            if (input[*i + 1] == '/') {
                while (input[*i] != '\n')
                    (*i)++;
                continue;
            } else if (input[*i + 1] == '*') {
                while (input[*i] != '*' || input[*i + 1] != '/')
                    (*i)++;
                *i += 2;
                continue;
            }
        }
        break;
    }
}

/*
 * Скипает typedef с его телом
 */
void processTypedef(const char *input, int *i, int inputSize) {
    // typedef long long int; или typedef struct { };
    while ((input[*i] != ';' || input[*i] != '{') && *i < inputSize)
        (*i)++;

    if (input[*i] == ';')
        return;
    else
        while (input[*i] != '}' && *i < inputSize) // Пропускаем полностью блок struct { }
            (*i)++;
}

/*
 * Проверка, это объявление структуры или объявление переменной типа struct
 */
bool checkStruct(const char *input, int *i, int inputSize) {
    // Создадим временную переменную для случая, если мы обознались и на самом деле struct - объявление переменной
    int j = *i;

    // Скипнем комментарии, потом имя структуры (даже если его нет), потом опять комментарии
    skip3(input, &j, inputSize);
    while (isalnum(input[j]) || input[j] == '_')
        j++;
    skip3(input, &j, inputSize);

    // Это объявление структуры? Если да, то должна быть {
    if (input[j] == '{')
        while (input[j] != '}' && j < inputSize) // Пропускаем полностью блок struct { }
            j++;
    else
        return false;

    *i = j;
    return true;
}

/*
 * Проверка, все ли переменные были инициализированы в строке? Если да, то нам не нужно проверять следующую строку
 */
bool checkVariables(VARIABLE *variables, int variableCount) {
    for (int i = 0; i < variableCount; ++i)
        if (!variables[i].isInitialized)
            return false;
    return true;
}

/*
 * Проверяет одну строчку на наличие объявления переменных. Так же по пути определяет, инициализированы ли переменные
 */
void checkFirstLine(const char *input, int inputSize, char *word, VARIABLE *variables, int *wordSize, int *i,
                    int *variableCount) {
    while (input[(*i)] != ';') {
        skip3(input, i, inputSize);
        // Собираем слово, полагая, что это имя переменной
        while (isalnum(input[(*i)]) || input[(*i)] == '_') {
            word[(*wordSize)++] = input[(*i)++];
        }

        // Это не переменная
        if (strlen(word) == 0) {
            break;
        }

        skip3(input, i, inputSize);

        // Массивы имеют начальное значение по умолчанию, не трогаем их
        if (input[(*i)] == '[') {
            break;
        }

        // Если мы встретили (, то это функция
        if (input[(*i)] == '(') {
            // Пропустим () функции
            int bracketSequence = 0;
            do {
                skip3(input, i, inputSize);
                if (input[(*i)] == '(') {
                    bracketSequence++;
                    (*i)++;
                    continue;
                }
                if (input[(*i)] == ')') {
                    bracketSequence--;
                    (*i)++;
                    continue;
                }
                (*i)++;
            } while (bracketSequence != 0);
            clearWord2(word, wordSize);
            break;
        }

        // Это не инициализация, запоминаем имя переменной в стек
        if (input[(*i)] != '=')
            strcpy(variables[(*variableCount)++].value, word);
        else
            while (input[(*i)] != ',' && input[(*i)] != ';')
                (*i)++;

        clearWord2(word, wordSize);
        // Мы пришли либо к ',', либо к ';'
    }
}

/*
 * Обработка scanf и fscanf. Там могут инициализироваться переменные
 */
void processScanfs(const char *input, int *t, int inputSize, VARIABLE *variables, int variableCount) {
    // Дойдём до переменных в scanf
    (*t)++;

    char word [WORDS] = {0};
    int wordSize = 0;

    while (input[*t] != ')') {
        skip3(input, t, inputSize);

        // Собираем слово, полагая, что это имя переменной
        while (isalnum(input[*t]) || input[*t] == '_') {
            word[wordSize++] = input[(*t)++];
        }

        // Ищем слово в словаре
        for (int k = 0; k < variableCount; ++k)
            if (!strcmp(variables[k].value, word))
                variables[k].isInitialized = true;

        clearWord2(word, &wordSize);
    }
}

/*
 * Проверяет вторую строчку, в которой возможно будет присвоение переменным значений
 */
void checkSecondLine(const char *input, int inputSize, char *word, int *wordSize, VARIABLE *variables,
                     int variableCount, int *t) {
    while (input[(*t)] != ';') {
        skip3(input, t, inputSize);

        // Собираем слово, полагая, что это имя переменной
        while (isalnum(input[(*t)]) || input[(*t)] == '_') {
            word[(*wordSize)++] = input[(*t)++];
        }

        // Может это scanf или fscanf?
        if (!strcmp(word, "scanf") || !strcmp(word, "fscanf")) {
            processScanfs(input, t, inputSize, variables, variableCount);
            clearWord2(word, wordSize);
            break;
        }

        // Это не переменная
        if (strlen(word) == 0) {
            break;
        }

        skip3(input, t, inputSize);

        // Это инициализация
        if (input[(*t)] == '=') {
            while (input[(*t)] != ',' && input[(*t)] != ';')
                (*t)++;
            for (int k = 0; k < variableCount; ++k)
                if (!strcmp(variables[k].value, word))
                    variables[k].isInitialized = true;
        }

        clearWord2(word, wordSize);
        // Мы пришли либо к ',', либо к ';'
    }
}

/*
 * Распечатывает неинициализированные переменные
 */
void printNotInitialized(const VARIABLE *variables, int variableCount) {
    for (int k = 0; k < variableCount; ++k)
        if (!variables[k].isInitialized)
            printf("A variable '%s' is not initialized\n", variables[k].value);
}

/*
 * Скипает длинные типы данных, например: long long int
 */
void skipTypes(char *input, int *i, int inputSize, stateTypes *now, int nowSize) {
    for (int j = 0; j < nowSize; ++j) {
        if (now[j].value == INIT && !strncmp(&input[(*i)], now[j].stateName, strlen(now[j].stateName))) {
            *i += (int)strlen(now[j].stateName);
            j = -1;
            skip3(input, i, inputSize);
        }
    }
}

/*
 * Очищает поле isInitialized структуры VARIABLE
 */
void clearVariables(VARIABLE *variables, int variableCount) {
    for (int i = 0; i < variableCount; ++i)
        variables[i].isInitialized = false;
}

/*
 * Скипает ТОЛЬКО комментарии. У меня были какие-то проблемы с функцией skip3
 */
void skipComments(const char *input, int inputSize, int *i) {
    if (input[(*i)] == '/' && input[(*i) + 1] == '/' || input[(*i)] == '/' && input[(*i)] == '*' || input[*i] == '"') {
        skip3(input, i, inputSize);
        --(*i);
    }
}

void checkInit(char *input, int inputSize, stateTypes *now, int nowSize) {
    char word [WORDS] = {0};
    int wordSize = 0;

    for (int i = 0; i < inputSize; ++i) {
        // Ищем слова
        if (isalnum(input[i]) || input[i] == '_') {
            word[wordSize++] = input[i];
        } else {
            // Пропуск комментариев
            skipComments(input, inputSize, &i);
            //skip3(input, &i, inputSize);

            // Пропуск typedef, struct
            if (!strcmp(word, "typedef")) {
                clearWord2(word, &wordSize);
                processTypedef(input, &i, inputSize);
                continue;
            }
            if (!strcmp(word, "struct")) {
                clearWord2(word, &wordSize);
                if (checkStruct(input, &i, inputSize))
                    continue;
            }

            for (int j = 0; j < nowSize && strlen(word) != 0; ++j) {
                if (!strcmp(now[j].stateName, word) && now[j].value == INIT && strlen(word) != 0) {
                    // Это стек, в которым мы будем заносить переменный временно
                    VARIABLE variables[NUMBER_OF_VARIABLES];
                    int variableCount = 0;

                    // Перед началом, скипнем всё ненужное и почислим слово
                    skip3(input, &i, inputSize);
                    clearWord2(word, &wordSize);

                    // Скипнем все другие типы данных типа long long int
                    skipTypes(input, &i, inputSize, now, nowSize);

                    // Пока не ';', будем пытаться взять переменные
                    checkFirstLine(input, inputSize, word, variables, &wordSize, &i, &variableCount);

                    // Возможно, у нас остались неинициализированные переменные (в массиве variables), проверим их
                    // Если все переменные инициализированы, то выходим
                    if(checkVariables(variables, variableCount))
                        break;

                    // Для своей безопасности, я буду анализировать следующую строку с временной переменной
                    int t = i;
                    t++;
                    skip3(input, &t, inputSize);

                    checkSecondLine(input, inputSize, word, &wordSize, variables, variableCount, &t);

                    // Закончили проверять. Выводим переменные, которые не инициализировались
                    printNotInitialized(variables, variableCount);

                    // Очищаем структуру VARIABLE. Из-за того что я не очищал, были баги
                    clearVariables(variables, variableCount);
                }
            }
            clearWord2(word, &wordSize);
        }
    }
}