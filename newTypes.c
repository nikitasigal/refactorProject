#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "wordHandler.h"
#include "newTypes.h"

#define WORDS 2000
#define TEXT_SIZE 100000
#define WORDS_FOR_STATE_NUM 19                                              //Комментарии на русском, т.к. читать будут все + Костя
//Всё, что можно вынести в функции обозначено FUNCT.

typedef enum {                                                              //Наши состояния
    FOR,                                                                    //Состояние FOR. Распостраняется на for, while, switch
    FOR_BODY,                                                               //Два состояния для отслежки стека циклов
    IF_BODY,
    FOR_SINGLE,
    IF_SINGLE,
    ELSE_BODY,
    ELSE_SINGLE,
    INIT,                                                                   //Состояние INIT. Распостраняется на int/char/bool/еще добавить
    TYPEDEF,                                                                //Состояние TYPEDEF. Оно почти юзлесс, но убирать его нельзя
    STRUCT,                                                                 //Состояние STRUCT. Не сильно отличается от INIT. (подр. см. 269)
    IF,                                                                     //Состояние IF. Работает точно, как FOR. Но, для отделения от FOR_BODY и FOR_SINGLE после, выделено отдельно.
    CASE,                                                                   //Состояние CASE. Работает пока одно.
    NOTHING,                                                                //Состояние NOTHING. Не пригодилось, но пока оставил.
    EMPTY,                                                                   //Начальное состояние.
    ELSE
} state;

struct Node {
    state value;
    struct Node *next;
};                                                          //До основной функции всё, что связано со стеком. Классика.

typedef struct {
    char stateName[WORDS];
    state value;
} stateTypes;


void clearWord(char *word, int *wordSize) {
    for (int l = 0; l < *wordSize; l++)
        word[l] = 0;
    (*wordSize) = 0;
}

//Typedef, struct, enum
void newTypes (stateTypes *now, int *nowSize, char *input, int inputSize) {
    char word [WORDS] = { 0 };
    int wordSize = 0;

    for (int i = 0; i < inputSize; ++i) {
        if (isalnum(input[i]) || input[i] == '_') {
            word[wordSize++] = input[i];
        } else {
            //TODO New, cooler skip
            if (!strcmp(word, "struct") || !strcmp(word, "enum")){
                clearWord(word, &wordSize);
                while (isalnum(input[i] || input[i] == '_'))
                    word[wordSize++] = input[i++];
                strcpy(now[*nowSize].stateName, word);
                now[*nowSize].value = INIT;
                (*nowSize)++;
            }

            if (!strcmp(word, "typedef")){
                int j = i;
                //TODO New, cooler skip
                if (!strncmp((&input)[j], "struct", 6*sizeof(char))){
                    while (input[j] != '}')
                        j++;
                    //TODO New, cooler skip
                    while (isalnum(input[i] || input[i] == '_'))
                        word[wordSize++] = input[i++];
                    strcpy(now[*nowSize].stateName, word);
                    now[*nowSize].value = INIT;
                    (*nowSize)++;
                }

            }



            clearWord(word, &wordSize);
        }
    }
}
