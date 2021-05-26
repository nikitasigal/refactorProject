#include "incorrectWriting.h"
#include "definitions.h"
#include "newTypes.h"

typedef struct {                         // Структура слово - состояние. Обычный массив, в который заносятся слова
    char stateName[WORD_LENGTH];
    state value;
    int line;
} wrongNameFull;

int isAlreadyInside(wrongNameFull *arr, int arrSize, char *name){
    for (int i = 0; i < arrSize; i++){
        if (!strcmp(name, arr[i].stateName)){
            return 1;
        }
    }
    return 0;
}

/*
 * Находим новые типы данных! struct, enum, typedef. Работает при любых случаях typedef.
 * Работает фактически втупую: встретил struct / enum -> взял его имя (если оно есть)
 * С typedef сложнее, ибо у него разные случаи: typedef long long int ll; или typedef struct A { } B;
 * typedef обрабатывается так: мы пропускаем весь ненужный нам текст. в случае struct и enum, нам надо найти, где }
 * и взять имя.
 * В ином случае, нам надо пропустить все типы данных (long long int и подобное) и взять имя
 */
void incorrectWriting(stateTypes *now, int *nowSize, int initialSize, char *input, int inputSize, char variables[][NAME_SIZE], int *variablesSize,
                                                                                                  char functions[][NAME_SIZE], int *functionsSize) {
    char word[WORD_LENGTH] = {0};
    int wordSize = 0;

    wrongNameFull sucks[WORDS_FOR_STATE_NUM]; //Массив переменных и функций, который сосут = написаны не по стилю
    int sucksSize = 0;                     //Размер этого массива

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

            //Дальше идёт проверка на правописание переменных\функций - Обращаться к Размику
            for (int l = 0; l < *nowSize; l++) {
                if (!strcmp(word, now[l].stateName) && (now[l].value == INIT || now[l].value == STRUCT)) {
                    //Смотрим встретили ли мы объявление, неважно какое, главное не typedef, коз он не объявляет переменные
                    int j;                  //Переменная, помогающая бегать
                    int newLineNumber = 1;
                    bool multi = false;     //Для объявления переменных через запятую
                    bool inFunc = false;

                    //В случае struct и enum делаем то же, что и в прошлом коммите
                    if (!strcmp(word, "struct") || !strcmp(word, "enum")) {
                        // Берём слово и пушаем его как новый тип данных
                        skip2(input, &i, inputSize, &lineNumber);
                        clearWord(word, &wordSize);
                        if (isalnum(input[i]) || input[i] == '_'){
                            while(isalnum(input[i]) || input[i] == '_'){
                                word[wordSize++] = input[i++];
                            }
                        }

                        if ((word[0] < 'A' || word[0] > 'Z') &&
                            /*(l >= initialSize || now[l].value == STRUCT) &&*/
                            strlen(word) != 0) {

                            if (!isAlreadyInside(sucks, sucksSize, word)) {
                                strcpy(sucks[sucksSize].stateName, word);
                                sucks[sucksSize].value = TYPEDEF;
                                sucks[sucksSize].line = lineNumber;
                                (sucksSize)++;
                            }

                        }

                        skip2(input, &i, inputSize, &lineNumber);
                    }

                    //Для продолжения анализа
                    j = i;
                    newLineNumber = lineNumber;

                    //Если у нас был enum или struct, объявление будет после }, поэтому и идём до него
                    if (input[j] == '{' && now[l].value == STRUCT) {
                        while (input[j] != '}') {
                            skip2(input, &j, inputSize, &newLineNumber);
                            j++;
                            if (input[j] == '\n') {
                                newLineNumber++;
                                j++;
                            }
                        }
                        j++;
                    }

                    //Пропускаем всё лишнее и начинаем собирать имя нашей переменной
                    clearWord(word, &wordSize);
                    skip2(input, &j, inputSize, &newLineNumber);

                    //Пока была запятая, продолжаем
                    while (!multi) {

                        //Пропускаем ненужное после запятой
                        skip2(input, &j, inputSize, &newLineNumber);

                        //Формирование слова
                        readWord(input, word, &wordSize, &j);

                        for (int k = 0; k < *nowSize; k++){
                            if (!strcmp(word, now[k].stateName)) {
                                inFunc = true;
                                break;
                            }
                        }

                        if (inFunc)
                            break;

                        //Пропускаем ненужное до следующего символа
                        skip2(input, &j, inputSize, &newLineNumber);

                        //Если это (, то мы объявили функцию
                        if (input[j] == '(') {
                            //Проверяем подходит ли под PascalStyle
                            //Нет - закидываем в список сосущих с состоянием FUNC
                            if ((word[0] < 'A' || word[0] > 'Z') && strlen(word) != 0) {
                                if (!isAlreadyInside(sucks, sucksSize, word)) {
                                    strcpy(sucks[sucksSize].stateName, word);
                                    sucks[sucksSize].value = FUNC;
                                    sucks[sucksSize].line = newLineNumber;
                                    (sucksSize)++;
                                }
                            }

                            bool exists = false;

                            for (int k = 0; k < (*functionsSize); k++){
                                if (!strcmp(functions[k], word)){
                                    exists = true;
                                    break;
                                }
                            }

                            if (!exists){
                                strcpy(functions[(*functionsSize)++], word);
                            }

                            multi = true;
                            continue;
                            //Что-либо иное, просто переменная
                        } else {

                            if ((word[0] < 'a' || word[0] > 'z') &&
                                /*l < initialSize && now[l].value != STRUCT && */
                                strlen(word) != 0) {

                                if (!isAlreadyInside(sucks, sucksSize, word)) {
                                    strcpy(sucks[sucksSize].stateName, word);
                                    sucks[sucksSize].value = INIT;
                                    sucks[sucksSize].line = newLineNumber;
                                    (sucksSize)++;
                                }

                            }/* else if ((word[0] < 'A' || word[0] > 'Z') &&
                                       (l >= initialSize || now[l].value == STRUCT) &&
                                       strlen(word) != 0) {

                                strcpy(sucks[sucksSize].stateName, word);
                                sucks[sucksSize].value = INIT;
                                sucks[sucksSize].line = newLineNumber;
                                (sucksSize)++;

                            }*/

                            bool exists = false;

                            for (int k = 0; k < (*variablesSize); k++){
                                if (!strcmp(variables[k], word)){
                                    exists = true;
                                    break;
                                }
                            }

                            if (!exists){
                                strcpy(variables[(*variablesSize)], word);
                                (*variablesSize)++;
                            }
                        }
                        //Проверяем есть ли запятая
                        //Нет - заканчиваем
                        if (input[j] != ',') {
                            multi = true;
                            //Да - продолжаем проверять объявленные переменные
                        } else {
                            j++;
                            clearWord(word, &wordSize);
                        }
                    }
                    break;
                } else if (!strcmp(word, "typedef")){
                    char name[WORD_LENGTH] = { 0 };
                    int nameSize = 0;

                    while (input[i] != ';' && input[i] != '{') {

                        if (isalnum(input[i]) || input[i] == '_') {
                            name[nameSize++] = input[i];
                        } else
                            clearWord(name, &nameSize);

                        skipComments(input, inputSize, &i, &lineNumber);

                        if (input[i] == ';' || input[i] == '{')
                            break;

                        i++;
                    }

                    skip2(input, &i, inputSize, &lineNumber);

                    if (input[i] == ';'){
                        if ((name[0] < 'A' || name[0] > 'Z') &&
                            /*(l >= initialSize || now[l].value == STRUCT) &&*/
                            strlen(name) != 0) {

                            strcpy(sucks[sucksSize].stateName, name);
                            sucks[sucksSize].value = TYPEDEF;
                            sucks[sucksSize].line = lineNumber;
                            (sucksSize)++;

                        }
                    } else if (input[i] == '{') {

                        int j = i;
                        int newLineNumber = lineNumber;

                        while (input[j] != '}') {
                            skipComments(input, inputSize, &i, &lineNumber);
                            j++;
                            if (input[j] == '\n') {
                                newLineNumber++;
                                j++;
                            }
                        }
                        j++;

                        skip2(input, &j, inputSize, &newLineNumber);

                        while (input[j] != ';') {

                            if (isalnum(input[j]) || input[j] == '_') {
                                name[nameSize++] = input[j];
                            } else
                                clearWord(name, &nameSize);
                            skipComments(input, inputSize, &i, &lineNumber);
                            j++;

                        }

                        if ((name[0] < 'A' || name[0] > 'Z') &&
                            /*(l >= initialSize || now[l].value == STRUCT) &&*/
                            strlen(name) != 0) {

                            if (!isAlreadyInside(sucks, sucksSize, word)) {
                                strcpy(sucks[sucksSize].stateName, name);
                                sucks[sucksSize].value = TYPEDEF;
                                sucks[sucksSize].line = newLineNumber;
                                (sucksSize)++;
                            }

                        }
                    }

                    clearWord(word, &wordSize);
                    break;
                }
            }
            clearWord(word, &wordSize);
        }
    }

    printf("\nIncorrectly named:\n");

    for (int i = 0; i < sucksSize; i++){
        if (sucks[i].value == INIT){
            printf("Line %d: The variable '%s' is named wrong\n", sucks[i].line, sucks[i].stateName);
        }
    }

    for (int i = 0; i < sucksSize; i++){
        if (sucks[i].value == FUNC){
            printf("Line %d: The function '%s' is named wrong\n", sucks[i].line, sucks[i].stateName);
        }
    }

    for (int i = 0; i < sucksSize; i++){
        if (sucks[i].value == TYPEDEF){
            printf("Line %d: The data type '%s' is named wrong\n", sucks[i].line, sucks[i].stateName);
        }
    }
}
