#include "generalFunctions.h"

/*
 * Чистит слово (обнуляет строку и размер)
 */
void clearWord(char *word, int *wordSize) {
    for (int l = 0; l < *wordSize; l++)
        word[l] = 0;
    (*wordSize) = 0;
}

/*
 * Читает слово, начиная с текущего символа
 */
void readWord(const char *input, char *word, int *wordSize, int *i) {
    while (isalnum(input[(*i)]) || input[(*i)] == '_') {
        word[(*wordSize)++] = input[(*i)++];
    }
}

// skip версии 3.0. Скипает ненужные знаки, не печатая их, как делает это skip 1.0
void universalSkip(const char *input, int *i, int inputSize, int *lineNumber) {
    while (*i < inputSize) {
        // Пропускаем пробелы, \n, \t
        if (input[*i] == ' ' || input[*i] == '\n' || input[*i] == '\t' || input[*i] == ',' || input[*i] == '*' ||
            input[*i] == '&') {

            if (lineNumber != NULL && input[*i] == '\n')
                (*lineNumber)++;
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
 * Скипает ТОЛЬКО комментарии. У меня были какие-то проблемы с функцией universalSkip
 */
void skipComments(const char *input, int inputSize, int *i, int *lineNumber) {
    if (input[(*i)] == '/' && input[(*i) + 1] == '/' || input[(*i)] == '/' && input[(*i)] == '*' || input[*i] == '"' ||
        input[*i] == '\'') {
        universalSkip(input, i, inputSize, lineNumber);
        --(*i);
    }
}