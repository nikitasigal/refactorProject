#include "checkInitialization.h"

/*
 * Скипает typedef с его телом
 */
void processTypedef(const char *input, int *i, int inputSize, int *lineNumber) {
    // typedef long long int; или typedef struct { };
    while (input[*i] != ';' && input[*i] != '{' && *i < inputSize) {
        if (input[*i] == '\n')
            (*lineNumber)++;
        (*i)++;
    }

    if (input[*i] == ';')
        return;
    else
        while (input[*i] != '}' && *i < inputSize) { // Пропускаем полностью блок struct { }
            if (input[*i] == '\n')
                (*lineNumber)++;
            (*i)++;
        }
}

/*
 * Проверка, это объявление структуры или объявление переменной типа struct
 */
bool checkStruct(const char *input, int *i, int inputSize, int *lineNumber) {
    // Создадим временную переменную для случая, если мы обознались и на самом деле struct - объявление переменной
    int j = *i;
    int tempLineNumber = *lineNumber;

    // Скипнем комментарии, потом имя структуры (даже если его нет), потом опять комментарии
    universalSkip(input, &j, inputSize, &tempLineNumber);
    while (isalnum(input[j]) || input[j] == '_')
        j++;
    universalSkip(input, &j, inputSize, &tempLineNumber);

    // Это объявление структуры? Если да, то должна быть {
    if (input[j] == '{')
        while (input[j] != '}' && j < inputSize) { // Пропускаем полностью блок struct { }
            j++;
            if (input[j] == '\n')
                (tempLineNumber)++;
        }
    else
        return false;

    *i = j;
    *lineNumber = tempLineNumber;
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
void checkFirstLine(char *input, int inputSize, char *word, VARIABLE *variables, int *wordSize, int *i,
                    int *variableCount, Map *functionsMap, Map *variablesMap, int *lineNumber, char *file,
                    stateTypes *now, int nowSize) {
    while (input[*i] != ';') {
        universalSkip(input, i, inputSize, lineNumber);

        // Собираем слово, полагая, что это имя переменной
        readWord(input, word, wordSize, i);

        // Это не переменная
        if (strlen(word) == 0)
            break;

        universalSkip(input, i, inputSize, lineNumber);

        // Массивы имеют начальное значение по умолчанию (ноль), не трогаем их
        if (input[*i] == '[')
            break;

        // Если мы встретили (, то это функция
        if (input[*i] == '(') {
            // Пушнем в мап функций. Функцию main не пушаем!!
            if (strcmp(word, "main") != 0)
                insertElement(functionsMap, word, *lineNumber, true, file);

            // Проверим, объявление или инициализация это. Если объявление, то не надо брать переменные в мап
            int j = *i;
            int tempBracketSequence = 0;
            do {
                universalSkip(input, i, inputSize, lineNumber);
                if (input[j] == '(') {
                    tempBracketSequence++;
                    j++;
                    continue;
                }
                if (input[j] == ')') {
                    tempBracketSequence--;
                    j++;
                    continue;
                }
                j++;
            } while (tempBracketSequence != 0);

            // Если дальше идёт ';', то это объявление
            if (input[j] == ';') {
                *i = j;
                clearWord(word, wordSize);
                break;
            }

            // Пропустим () функции
            int bracketSequence = 0;
            do {
                if (input[*i] == '(') {
                    bracketSequence++;
                    (*i)++;
                    continue;
                }
                if (input[*i] == ')') {
                    bracketSequence--;
                    (*i)++;
                    continue;
                }
                if (input[*i] == '[') {
                    while (input[*i] != ']')
                        (*i)++;
                    (*i)++;
                }
                clearWord(word, wordSize);

                // Пропустим лишнее
                skipTypes(input, i, inputSize, now, nowSize, lineNumber);
                universalSkip(input, i, inputSize, lineNumber);
                skipTypes(input, i, inputSize, now, nowSize, lineNumber);
                universalSkip(input, i, inputSize, lineNumber);

                // Возьмём имя переменной
                readWord(input, word, wordSize, i);

                // Пушаем переменную в мап
                if (strlen(word) != 0)
                    insertElement(variablesMap, word, *lineNumber, false, file);
                variablesMap[hash(word)].isInit = true;
            } while (bracketSequence != 0);
            clearWord(word, wordSize);
            break;
        }

        // Это не инициализация, запоминаем имя переменной в стек
        if (input[*i] != '=') {
            variables[*variableCount].line = *lineNumber;
            strcpy(variables[(*variableCount)++].value, word);
        } else {
            (*i)++;
            universalSkip(input, i, inputSize, lineNumber);

            // Пропустим { }
            if (input[*i] == '{') {
                int bracesSequence = 0;
                do {
                    if (input[*i] == '{') {
                        bracesSequence++;
                        (*i)++;
                        continue;
                    }
                    if (input[*i] == '}') {
                        bracesSequence--;
                        (*i)++;
                        continue;
                    }
                    (*i)++;
                } while (bracesSequence != 0);
                universalSkip(input, i, inputSize, lineNumber);
            }
            while (input[*i] != ',' && input[*i] != ';')
                (*i)++;
        }

        // Пушаем переменную в мап
        insertElement(variablesMap, word, *lineNumber, false, file);

        clearWord(word, wordSize);
        // Мы пришли либо к ',', либо к ';'
    }
}

/*
 * Обработка scanf и fscanf. Там могут инициализироваться переменные
 */
void processScanfs(const char *input, int *t, int inputSize, VARIABLE *variables, int variableCount, int *lineNumber) {
    // Дойдём до переменных в scanf
    (*t)++;

    char word[WORD_LENGTH] = {0};
    int wordSize = 0;

    while (input[*t] != ')') {
        universalSkip(input, t, inputSize, lineNumber);

        // Собираем слово, полагая, что это имя переменной
        readWord(input, word, &wordSize, t);

        // Ищем слово в словаре
        for (int k = 0; k < variableCount; ++k)
            if (!strcmp(variables[k].value, word))
                variables[k].isInitialized = true;

        clearWord(word, &wordSize);
    }
}

/*
 * Проверяет вторую строчку, в которой возможно будет присвоение переменным значений
 */
void checkSecondLine(const char *input, int inputSize, char *word, int *wordSize, VARIABLE *variables,
                     int variableCount, int *t, int *lineNumber) {
    while (input[*t] != ';') {
        universalSkip(input, t, inputSize, lineNumber);

        // Собираем слово, полагая, что это имя переменной
        readWord(input, word, wordSize, t);

        // Может это scanf или fscanf?
        if (!strcmp(word, "scanf") || !strcmp(word, "fscanf")) {
            processScanfs(input, t, inputSize, variables, variableCount, lineNumber);
            clearWord(word, wordSize);
            break;
        }

        // Это не переменная
        if (strlen(word) == 0)
            break;

        universalSkip(input, t, inputSize, lineNumber);

        // Это инициализация
        if (input[*t] == '=') {
            while (input[*t] != ',' && input[*t] != ';')
                (*t)++;
            for (int k = 0; k < variableCount; ++k)
                if (!strcmp(variables[k].value, word))
                    variables[k].isInitialized = true;
        }

        clearWord(word, wordSize);
        // Мы пришли либо к ',', либо к ';'
    }
}

/*
 * Скипает длинные типы данных, например: long long int
 */
void skipTypes(char *input, int *i, int inputSize, stateTypes *now, int nowSize, int *lineNumber) {
    for (int j = 0; j < nowSize; ++j) {
        if ((now[j].value == INIT || now[j].value == STRUCT) && !strncmp(&input[(*i)], now[j].stateName, strlen(now[j].stateName)) && !isalnum(input[(*i) + strlen(now[j].stateName)])) {
            *i += (int) strlen(now[j].stateName);
            j = -1;
            universalSkip(input, i, inputSize, lineNumber);
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
 * Проверка на инициализацию
 */
void checkInit(char *input, int inputSize, stateTypes *now, int nowSize, Map *variablesMap, Map *functionsMap,
               Map *variablesInitMap, int *lineNumber, char *file) {
    char word[WORD_LENGTH] = {0};
    int wordSize = 0;

    for (int i = 0; i < inputSize; ++i) {
        // Ищем слова
        if (isalnum(input[i]) || input[i] == '_') {
            word[wordSize++] = input[i];
        } else {
            if (input[i] == '\n')
                (*lineNumber)++;

            // Пропуск комментариев
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

            for (int j = 0; j < nowSize && strlen(word) != 0; ++j) {
                if (!strcmp(now[j].stateName, word) && now[j].value == INIT) {
                    // Это стек, в которым мы будем заносить переменный временно
                    VARIABLE variables[NUMBER_OF_VARIABLES];
                    int variableCount = 0;

                    // Перед началом, скипнем всё ненужное и почислим слово
                    universalSkip(input, &i, inputSize, lineNumber);
                    clearWord(word, &wordSize);

                    // Скипнем все другие типы данных типа long long int
                    skipTypes(input, &i, inputSize, now, nowSize, lineNumber);

                    // Пока не ';', будем пытаться взять переменные
                    checkFirstLine(input, inputSize, word, variables, &wordSize, &i, &variableCount, functionsMap,
                                   variablesMap, lineNumber, file, now, nowSize);

                    // Возможно, у нас остались неинициализированные переменные (в массиве variables), проверим их
                    // Если все переменные инициализированы, то выходим
                    if (checkVariables(variables, variableCount))
                        break;

                    // Для своей безопасности, я буду анализировать следующую строку с временной переменной
                    int t = i;
                    t++;
                    skipComments(input, inputSize, &t, NULL);

                    // Проверяем следующую строку после инициализации. Возможно, там переменные получают начальные значения
                    checkSecondLine(input, inputSize, word, &wordSize, variables, variableCount, &t, NULL);

                    // Закончили проверять. Выводим переменные, которые не инициализировались
                    for (int k = 0; k < variableCount; ++k)
                        if (!variables[k].isInitialized)
                            insertElement(variablesInitMap, variables[k].value, variables[k].line, false, file);

                    // Очищаем структуру VARIABLE. Из-за того что я не очищал, были баги
                    clearVariables(variables, variableCount);
                }
            }
            clearWord(word, &wordSize);
        }
    }

    // Обработали один файл. Обнулим (объединичим) количество номер строки
    *lineNumber = 1;
}