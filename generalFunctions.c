#include "generalFunctions.h"

// Очистка слова
void clearWord(char *word, int *wordSize) {
    for (int l = 0; l < *wordSize; l++)
        word[l] = 0;
    (*wordSize) = 0;
}

void readWord(const char *input, char *word, int *wordSize, int *i) {
    while (isalnum(input[(*i)]) || input[(*i)] == '_') {
        word[(*wordSize)++] = input[(*i)++];
    }
}