#include "checkLooping.h"

void skip3_2(const char *input, int *i, int inputSize, int *lineNumber) {
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
            while ((input[*i - 1] == '\\' && input[*i - 2] != '\\') || input[*i] != '\'') {
                (*i)++;
                if (input[*i] == '\n')
                    (*lineNumber)++;
            }
            (*i)++;
            continue;
        }
        if (input[*i] == '"') {
            (*i)++;
            while (input[*i - 1] == '\\' || input[*i] != '"') {
                (*i)++;
                if (input[*i] == '\n')
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

void skipComments_2(const char *input, int inputSize, int *i, int *lineNumber) {
    if (input[(*i)] == '/' && input[(*i) + 1] == '/' || input[(*i)] == '/' && input[(*i)] == '*' || input[*i] == '"' ||
        input[*i] == '\'' || input[*i] == '*') {
        skip3_2(input, i, inputSize, lineNumber);
        if (isalnum(input[*i]) || input[*i] == '_')
            (*i)--;
    }
}

/* DISCLAIMER
 * Данный код является происками дьявола и не содержит в себе ни логического, ни духовного смысла.
 * Если он выдаёт неверный результат, обратитесь к Размику, он закинет вам ещё один костыль
*/

void checkLooping(char *input, int inputSize, char variables[][NAME_SIZE], const int *variablesSize) {
    char word[WORD_LENGTH] = {0};
    int wordSize = 0;

    int lineNumber = 1;

    whileStats search[WHILE_LENGTH];
    int searchSize = 0;

    for (int i = 0; i < inputSize; ++i) {
        if (isalnum(input[i]) || input[i] == '_') {
            word[wordSize++] = input[i];
        } else {
            if (input[i] == '\n')
                lineNumber++;

            skipComments_2(input, inputSize, &i, &lineNumber);

            if (strlen(word) == 0)
                continue;

            if (!strcmp(word, "while")) {
                bool infiniteCondition = false;
                char condition[WORD_LENGTH] = {0};
                bool exclamationNum = false;
                int conditionSize = 0;
                int bracketSequence = 0;

                if (input[i] == ' ')
                    i++;

                search[searchSize].Looped = false;
                search[searchSize].lineNum = lineNumber;

                while (((input[i - 1] != ')') || (bracketSequence != 0)) && i < inputSize) {
                    skipComments_2(input, inputSize, &i, &lineNumber);

                    if (input[i] == ' ')
                        infiniteCondition = true;

                    if (input[i] == '(')
                        bracketSequence++;
                    else if (input[i] == ')')
                        bracketSequence--;


                    if (isalnum(input[i]) || input[i] == '_') {
                        condition[conditionSize++] = input[i];
                    } else {
                        if (input[i] == '!')
                            exclamationNum = !exclamationNum;

                        if (input[i] == '\n')
                            lineNumber++;

                        for (int j = 0; j < *variablesSize; j++) {
                            if (!strcmp(condition, variables[j])) {
                                strcpy(search[searchSize].variableNames[search[searchSize].variableNamesSize++],
                                       variables[j]);
                                search[searchSize].Looped = true;
                                infiniteCondition = false;
                            }
                        }

                        if (input[i] != ')')
                            clearWord(condition, &conditionSize);
                    }
                    i++;
                }
                i++;

                if (input[i] == '\n')
                    lineNumber++;

                if (((!strcmp(condition, "false") && !exclamationNum) ||
                     (!strcmp(condition, "0") && !exclamationNum) ||
                     (!strcmp(condition, "true") && exclamationNum) ||
                     (!strcmp(condition, "1") && exclamationNum)) &&
                    !infiniteCondition)
                    infiniteCondition = true;


                if (!infiniteCondition)
                    search[searchSize].Looped = true;

                searchSize++;

                if (input[i] == ' ')
                    i++;

                skipComments_2(input, inputSize, &i, &lineNumber);

                int j = i;
                int newBracketSequence = 0;

                if (input[j] == ' ')
                    j++;

                skipComments_2(input, inputSize, &j, &lineNumber);

                if (input[j] == '{') {
                    newBracketSequence++;
                    j++;

                    while (((input[j - 1] != '}') || (newBracketSequence != 0)) && j < inputSize) {
                        skipComments_2(input, inputSize, &i, &lineNumber);

                        if (input[j] == '{')
                            newBracketSequence++;
                        else if (input[j] == '}')
                            newBracketSequence--;

                        if (isalnum(input[j]) || input[j] == '_') {
                            condition[conditionSize++] = input[j];
                        } else {

                            for (int k = 0; k < searchSize; k++)
                                for (int l = 0; l < search[searchSize - 1].variableNamesSize; l++)
                                    if (!strcmp(condition, search[searchSize - 1].variableNames[l])) {
                                        search[searchSize - 1].Looped = false;
                                        break;
                                    }

                            if ((!(strcmp(condition, "break")))) {
                                search[searchSize - 1].Looped = false;
                                break;
                            }

                            clearWord(condition, &conditionSize);
                        }
                        j++;
                    }
                } else {
                    while ((((input[j - 1] != ';') && (input[j - 1] != '}')) || (newBracketSequence != 0)) && j < inputSize) {
                        if (input[j] == '{')
                            newBracketSequence++;
                        else if (input[j] == '}')
                            newBracketSequence--;

                        if (isalnum(input[j]) || input[j] == '_') {
                            condition[conditionSize++] = input[j];
                        } else {
                            for (int k = 0; k < searchSize; k++) {
                                for (int l = 0; l < search[searchSize - 1].variableNamesSize; l++) {
                                    if (!strcmp(condition, search[searchSize - 1].variableNames[l])) {
                                        search[searchSize - 1].Looped = false;
                                        break;
                                    }
                                }
                            }

                            if ((!(strcmp(condition, "break")))) {
                                search[searchSize - 1].Looped = false;
                                break;
                            }

                            clearWord(condition, &conditionSize);
                        }
                        j++;
                    }
                }

            } else if (!strcmp(word, "for")) {
                char condition[WORD_LENGTH] = {0};
                int conditionSize = 0;
                int bracketSequence = 0;
                int progressState = 0;

                if (input[i] == ' ')
                    i++;

                search[searchSize].Looped = false;
                search[searchSize].lineNum = lineNumber;

                while (((input[i - 1] != ')') || (bracketSequence != 0)) && i < inputSize) {

                    skipComments_2(input, inputSize, &i, &lineNumber);

                    if (input[i] == ';')
                        progressState++;

                    if (input[i] == '(')
                        bracketSequence++;
                    else if (input[i] == ')')
                        bracketSequence--;

                    if (isalnum(input[i]) || input[i] == '_') {
                        condition[conditionSize++] = input[i];
                    } else {

                        if (input[i] == '\n')
                            lineNumber++;

                        if (progressState == 2 && strlen(condition)) {
                            for (int j = 0; j < search[searchSize].variableNamesSize; j++) {
                                if (!strcmp(condition,
                                            search[searchSize].variableNames[j])) {
                                    search[searchSize].Looped = false;
                                }
                            }
                        } else {

                            for (int j = 0; j < *variablesSize; j++) {
                                if (!strcmp(condition, variables[j])) {
                                    strcpy(search[searchSize].variableNames[search[searchSize].variableNamesSize++],
                                           variables[j]);
                                    search[searchSize].Looped = true;
                                }
                            }

                        }

                        if (input[i] != ')')
                            clearWord(condition, &conditionSize);

                    }
                    i++;
                }
                i++;

                progressState = 0;

                /*if (input[i] == '\n')
                    lineNumber++;*/

                if (search[searchSize].variableNamesSize == 0){
                    search[searchSize].Looped = true;
                }

                searchSize++;

                if (search[searchSize - 1].Looped == false) {
                    i--;
                    clearWord(word, &wordSize);
                    continue;
                }
                else {
                    if (input[i] == ' ')
                        i++;

                    skipComments_2(input, inputSize, &i, &lineNumber);

                    int j = i;
                    int newBracketSequence = 0;

                    if (input[j] == ' ')
                        j++;

                    skipComments_2(input, inputSize, &j, &lineNumber);

                    if (input[j] == '{') {
                        newBracketSequence++;
                        j++;

                        while (((input[j - 1] != '}') || (newBracketSequence != 0)) && j < inputSize) {
                            skipComments_2(input, inputSize, &i, &lineNumber);

                            if (input[j] == '{')
                                newBracketSequence++;
                            else if (input[j] == '}')
                                newBracketSequence--;

                            if (isalnum(input[j]) || input[j] == '_') {
                                condition[conditionSize++] = input[j];
                            } else {

                                for (int k = 0; k < searchSize; k++)
                                    for (int l = 0; l < search[searchSize - 1].variableNamesSize; l++)
                                        if (!strcmp(condition, search[searchSize - 1].variableNames[l])) {
                                            search[searchSize - 1].Looped = false;
                                            break;
                                        }

                                if ((!(strcmp(condition, "break")))) {
                                    search[searchSize - 1].Looped = false;
                                    break;
                                }

                                clearWord(condition, &conditionSize);
                            }
                            j++;
                        }

                    } else {
                        while ((((input[j - 1] != ';') && (input[j - 1] != '}')) || (newBracketSequence != 0)) && j < inputSize) {
                            if (input[j] == '{')
                                newBracketSequence++;
                            else if (input[j] == '}')
                                newBracketSequence--;

                            if (isalnum(input[j]) || input[j] == '_') {
                                condition[conditionSize++] = input[j];
                            } else {
                                for (int k = 0; k < searchSize; k++) {
                                    for (int l = 0; l < search[searchSize - 1].variableNamesSize; l++) {
                                        if (!strcmp(condition, search[searchSize - 1].variableNames[l])) {
                                            search[searchSize - 1].Looped = false;
                                            break;
                                        }
                                    }
                                }

                                if ((!(strcmp(condition, "break")))) {
                                    search[searchSize - 1].Looped = false;
                                    break;
                                }

                                clearWord(condition, &conditionSize);
                            }
                            j++;
                        }
                    }
                }
            }
        clearWord(word, &wordSize);
        }
    }
    printf("\nPossible looping:\n");

    for (int i = 0; i < searchSize; i++)
        if (search[i].Looped)
            printf("Line %d: possible infinity\n", search[i].lineNum);
}