#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "newTypes.h"

// Очистка слова
void clearWord(char *word, int *wordSize) {
    for (int l = 0; l < *wordSize; l++)
        word[l] = 0;
    (*wordSize) = 0;
}

// skip версии 2.0. Скипает пробелы, \n, \t и комментарии, не печатая их
void skip2(const char *input, int *i, int inputSize) {
    while (*i < inputSize) {
        // Пропускаем пробелы, \n, \t
        if (input[*i] == ' ' || input[*i] == '\n' || input[*i] == '\t') {
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

// Пушает новый тип данных в массив stateTypes, очищая
void pushNewType(stateTypes *now, int *nowSize, const char *input, char *word, int *wordSize, int *i) {
    // Чистим слово от мусора / предыдущего слова
    clearWord(word, wordSize);

    // Собираем слово
    while (isalnum(input[(*i)]) || input[(*i)] == '_')
        word[(*wordSize)++] = input[(*i)++];

    // Проверим, нет ли этого слова в массиве
    for (int j = 0; j < *nowSize; ++j) {
        if (!strcmp(word, now[j].stateName) && now[j].value == INIT) {
            // Чистим слово от мусора / предыдущего слова
            clearWord(word, wordSize);
            return;
        }
    }

    // Добавляем новое слово, если оно не пустое
    if (strlen(word) != 0) {
        strcpy(now[*nowSize].stateName, word);
        now[*nowSize].value = INIT;
        (*nowSize)++;
    }
}

/*
 * Находим новые типы данных! struct, enum, typedef. Работает при любых случаях typedef.
 * Работает фактически втупую: встретил struct / enum -> взял его имя (если оно есть)
 * С typedef сложнее, ибо у него разные случаи: typedef long long int ll; или typedef struct A { } B;
 * typedef обрабатывается так: мы пропускаем весь ненужный нам текст. в случае struct и enum, нам надо найти, где }
 * и взять имя.
 * В ином случае, нам надо пропустить все типы данных (long long int и подобное) и взять имя
 */
void newTypes (stateTypes *now, int *nowSize, char *input, int inputSize) {
    char word [WORDS] = {0};
    int wordSize = 0;

    // Стандартные типы данных, которые нам стоит пропустить при typedef, например при:
    // typedef unsigned long long int a; - нам надо пропустить unsigned long long int, чтобы добраться до имени
    char specialWords[SPECIAL_WORDS_NUMBER][WORDS] = {"struct", "enum", "long",
                                                      "unsigned", "int", "double",
                                                      "char", "void", "short"};

    for (int i = 0; i < inputSize; ++i) {
        if (isalnum(input[i]) || input[i] == '_') {
            word[wordSize++] = input[i];
        } else {
            skip2(input, &i, inputSize);

            // Не надо нам пустые слова обрабатывать. Могут возникнуть ошибки, при strcmp
            if (strlen(word) == 0)
                continue;

            if (!strcmp(word, "struct") || !strcmp(word, "enum")) {
                // Берём слово и пушаем его как новый тип данных
                skip2(input, &i, inputSize);
                pushNewType(now, nowSize, input, word, &wordSize, &i);
            }

            // TODO Вообще, можно объединить эти два ифа

            if (!strcmp(word, "typedef")) {
                // typedef может сопровождаться { }, а может и нет. Здесь мы создаём временную переменную j, чтобы
                // вручную проверить, ГДЕ находится имя нового типа, и добавить его
                int j = i;
                skip2(input, &j, inputSize);

                // struct и enum могут сопровождаться именами даже при typedef, например:
                /*
                 * typedef struct A {
                 *      int a;
                 * } B;
                 *
                 * В таком случае, чтобы добраться до 'B', нам надо пропустить struct, enum, а также имя 'A'. Когда
                 * мы пропустим имя 'A', то встретим { и пойдём искать }, а после возьмём имя 'B'
                 */
                bool isStructOrEnum = false;
                if (!strncmp(&input[j], "struct", 6 * sizeof(char))) {
                    j += 6;
                    isStructOrEnum = true;
                    skip2(input, &j, inputSize);
                }
                if (!strncmp(&input[j], "enum", 4 * sizeof(char))) {
                    j += 4;
                    isStructOrEnum = true;
                    skip2(input, &j, inputSize);
                }

                // Скипаем слово после struct / enum (то есть имя структуры по сути), которое потом захватит другой if("struct") выше
                while (isStructOrEnum && (isalnum(input[j]) || input[j] == '_'))
                    j++;

                skip2(input, &j, inputSize);

                // Скипаем все типы данных, которые хотим пропустить
                for (int k = 0; k < SPECIAL_WORDS_NUMBER; ++k) {
                    if (!strncmp(&input[j], specialWords[k], strlen(specialWords[k]))) {
                        j += (int) strlen(specialWords[k]);
                        k = -1;
                        skip2(input, &j, inputSize);
                    }
                }

                // Если встретили {, то это инициализация структуры / перечисления. Ищем конец инициализации и берём имя
                if (input[j] == '{') {
                    while (input[j - 1] != '}')
                        ++j;
                    skip2(input, &j, inputSize);
                }

                // Берём слово и пушаем его как новый тип данных
                pushNewType(now, nowSize, input, word, &wordSize, &j);
            }

            clearWord(word, &wordSize);
        }
    }
}