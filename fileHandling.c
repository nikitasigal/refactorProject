#include "fileHandling.h"

void swapTexts(char *sourceText, int *sourceSize, char *outputText, int *outputSize) {
    strcpy(sourceText, outputText);
    *sourceSize = *outputSize;
    *outputSize = 0;
}

void readFileNames(char files[][WORD_LENGTH], int *fileCount) {
    // Директория файлов
    DIR *dir = opendir(SOURCE_DIRECTORY);
    if (dir == NULL)
        exit(EXIT_FAILURE);

    // Текущий файл
    struct dirent *ent = NULL;

    // Считываем все имена файлов в массив
    while ((ent = readdir(dir)))
        if (ent->d_namlen > 2) {
            char directory[WORD_LENGTH] = {0};
            strcat(directory, ent->d_name);
            strcpy(files[(*fileCount)++], directory);
        }

    // Закрываем директорию
    closedir(dir);
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