#include "unusedTokens.h"

/*
 * Аргументы функций до сего момента игнорировались. Пора это исправить и взять их под опеку
 */
void getArguments(char *input, int *i, int inputSize, stateTypes *now, int nowSize, Map *variablesMap, int *lineNumber) {
    // Сюда собираем текущее слово
    char word[WORDS] = {0};
    int wordSize = 0;

    // Сейчас мы на (, пропустим её
    (*i)++;

    while (input[*i] != ')') {
        skip3(input, i, inputSize, lineNumber);

        // Скипаем типы данных
        skipTypes(input, i, inputSize, now, nowSize, lineNumber);

        // Собираем слово, имя переменной
        while (isalnum(input[*i]) || input[*i] == '_') {
            word[wordSize++] = input[(*i)++];
        }

        // Добавляем её в мап
        insertElement(variablesMap, word, *lineNumber);

        clearWord(word, &wordSize);
    }
}

/*
 * Проверка на неиспользованные переменные и функции
 */
void checkUnused(char *input, int inputSize, stateTypes *now, int nowSize, Map *variablesMap, Map *functionsMap, int *lineNumber) {
    // Сюда собираем текущее слово
    char word[WORDS] = {0};
    int wordSize = 0;

    for (int i = 0; i < inputSize; ++i) {
        if (isalnum(input[i]) || input[i] == '_') {
            word[wordSize++] = input[i];
        } else {
            // Пропуск комментариев и " ", ' '
            skipComments(input, inputSize, &i, lineNumber);

            // Пропуск typedef, struct
            if (!strcmp(word, "typedef")) {
                clearWord(word, &wordSize);
                processTypedef(input, &i, inputSize, lineNumber);
                continue;
            }
            if (!strcmp(word, "struct")) {
                clearWord(word, &wordSize);
                if (checkStruct(input, &i, inputSize, lineNumber))
                    continue;
            }

            // Дальше пойдёт инициализация, или нет?
            bool wasInitialization = false;
            for (int j = 0; j < nowSize && strlen(word) != 0; ++j) {
                if (!strcmp(now[j].stateName, word) && now[j].value == INIT) {
                    // Перед началом, скипнем всё ненужное и почислим слово
                    skip3(input, &i, inputSize, lineNumber);
                    clearWord(word, &wordSize);

                    // Скипнем все другие типы данных типа long long int
                    skipTypes(input, &i, inputSize, now, nowSize, lineNumber);

                    // Берём имя переменной / структуры / функции
                    readWord(input, word, &wordSize, &i);

                    skip3(input, &i, inputSize, lineNumber);
                    clearWord(word, &wordSize);

                    // Если мы встретили (, то это функция. Возьмём её аргументы (так как у нас нет их в мапах)
                    if (strlen(word) != 0 && input[i] == '(')
                        getArguments(input, &i, inputSize, now, nowSize, variablesMap, lineNumber);

                    wasInitialization = true;
                }
            }

            // Если это была не инициализация, то возможно просто использование функции / переменной
            if (!wasInitialization && strlen(word) != 0) {
                checkElement(variablesMap, word);
                checkElement(functionsMap, word);
            }

            clearWord(word, &wordSize);
        }
    }

    printFooMap(functionsMap);
    printVarMap(variablesMap);
}
