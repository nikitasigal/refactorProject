#include "newTypes.h"

// skip версии 2.0. Скипает пробелы, \n, \t и комментарии, не печатая их
void skip2(const char *input, int *i, int inputSize, int *lineNumber) {
    while (*i < inputSize) {
        // Пропускаем пробелы, \n, \t
        if (input[*i] == ' ' || input[*i] == '\n' || input[*i] == '\t') {
            if (input[*i] == '\n'){
                (*lineNumber)++;
            }
            (*i)++;
            continue;
        }

        // Пропускаем комментарии
        if (input[*i] == '/') {
            if (input[*i + 1] == '/') {
                while (input[*i] != '\n')
                    (*i)++;
                if (input[*i] == '\n'){
                    (*lineNumber)++;
                }
                (*i)++;
                continue;
            } else if (input[*i + 1] == '*') {
                while (input[*i] != '*' || input[*i + 1] != '/') {
                    if (input[*i] == '\n'){
                        (*lineNumber)++;
                    }
                    (*i)++;
                }
                *i += 2;
                continue;
            }
        }

        // Пропускаем #
        if (input[*i] == '#')
            while (input[*i] != '\n')
                (*i)++;
        if (input[*i] == '\n'){
            (*lineNumber)++;
            (*i)++;
        }
        break;
    }
    /*if (isalnum(input[*i]) || input[*i] == '_')
        (*i)--;*/
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

void skipSpecialWords(const char *input, int inputSize, int *j, stateTypes *now, int nowSize) {
    int ln = 1;
    for (int k = 0; k < nowSize; ++k) {
        if (!strncmp(&input[(*j)], now[k].stateName, strlen(now[k].stateName)) &&
            (now[k].value == STRUCT || now[k].value == INIT)) {
            (*j) += (int) strlen(now[k].stateName);
            k = -1;
            skip2(input, j, inputSize, &ln);
        }
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
void newTypes(stateTypes *now, int *nowSize, int initialSize, char *input, int inputSize) {
    char word[WORDS] = {0};
    int wordSize = 0;

    int lineNumber = 1;

    for (int i = 0; i < inputSize; ++i) {
        if (isalnum(input[i]) || input[i] == '_') {
            word[wordSize++] = input[i];
        } else {
            skip2(input, &i, inputSize, &lineNumber);

            if (isalnum(input[i]) || input[i] == '_')
                (i)--;

            // Не надо нам пустые слова обрабатывать. Могут возникнуть ошибки, при strcmp
            if (strlen(word) == 0)
                continue;

            //Немного изменю вот это
            if (!strcmp(word, "struct") || !strcmp(word, "enum")) {
                // Берём слово и пушаем его как новый тип данных
                skip2(input, &i, inputSize, &lineNumber);
                pushNewType(now, nowSize, input, word, &wordSize, &i);
            }

            // TODO Вообще, можно объединить эти два ифа

            if (!strcmp(word, "typedef")) {
                // typedef может сопровождаться { }, а может и нет. Здесь мы создаём временную переменную j, чтобы
                // вручную проверить, ГДЕ находится имя нового типа, и добавить его
                int j = i;
                skip2(input, &j, inputSize, &lineNumber);

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
                    skip2(input, &j, inputSize, &lineNumber);
                }
                if (!strncmp(&input[j], "enum", 4 * sizeof(char))) {
                    j += 4;
                    isStructOrEnum = true;
                    skip2(input, &j, inputSize, &lineNumber);
                }

                // Скипаем слово после struct / enum (то есть имя структуры по сути), которое потом захватит другой if("struct") выше
                while (isStructOrEnum && (isalnum(input[j]) || input[j] == '_'))
                    j++;

                skip2(input, &j, inputSize, &lineNumber);

                // Скипаем все типы данных, которые хотим пропустить
                skipSpecialWords(input, inputSize, &j, now, *nowSize);

                // Если встретили {, то это инициализация структуры / перечисления. Ищем конец инициализации и берём имя
                if (input[j] == '{') {
                    while (input[j - 1] != '}')
                        ++j;
                    skip2(input, &j, inputSize, &lineNumber);
                }

                // Берём слово и пушаем его как новый тип данных
                pushNewType(now, nowSize, input, word, &wordSize, &j);
            }

        }
    }
}
