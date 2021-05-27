#include "unusedTokens.h"

/*
 * Проверка на неиспользованные переменные и функции
 */
void checkUnused(char *input, int inputSize, stateTypes *now, int nowSize, Map *variablesMap, Map *functionsMap,
                 int *lineNumber) {
    // Сюда собираем текущее слово
    char word[WORD_LENGTH] = {0};
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
                    universalSkip(input, &i, inputSize, lineNumber);
                    clearWord(word, &wordSize);

                    // Скипнем все другие типы данных типа long long int
                    skipTypes(input, &i, inputSize, now, nowSize, lineNumber);

                    // Берём имя переменной / структуры / функции
                    readWord(input, word, &wordSize, &i);

                    universalSkip(input, &i, inputSize, lineNumber);

                    if (strlen(word) != 0 && input[i] == '(') {
                        while(input[i] != '{' && input[i] != ';')
                            i++;
                        wasInitialization = true;
                        break;
                    }

                    if (input[i] == ';') {
                        wasInitialization = true;
                        clearWord(word, &wordSize);
                        break;
                    }

                    do {
                        i++;
                        wasInitialization = true;
                    } while(input[i] != ';');

                    clearWord(word, &wordSize);
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
}
