#include <stdio.h>
#include <string.h>
#include "specialSymbols.h"
#include "wordHandler.h"

#define TEXT_SIZE 100000

void swapTexts(char *sourceText, int *sourceSize, char *outputText, int *outputSize) {
    strcpy(sourceText, outputText);
    *sourceSize = *outputSize;
    *outputSize = 0;
}

int main() {
    FILE *sourceFile = fopen("input.c", "rt");
    char sourceText[TEXT_SIZE] = {0};
    int sourceSize = 0;
    while (!feof(sourceFile))
        sourceText[sourceSize++] = (char) getc(sourceFile);
    fclose(sourceFile);
    sourceSize--;

    char outputText[TEXT_SIZE] = {0};
    int outputSize = 0;

    // Formatting
    // Step 1 - special symbols
    processSpecialSymbols(sourceText, sourceSize, outputText, &outputSize);

    swapTexts(sourceText, &sourceSize, outputText, &outputSize);

    wordHandler(sourceText, sourceSize, outputText, &outputSize);

    // Formatting final - output new code
    FILE *outputFile = fopen("output.c", "wt");
    for (int i = 0; i < outputSize; ++i)
        putc(outputText[i], outputFile);
    fclose(outputFile);
}