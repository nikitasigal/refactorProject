#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "wordHandler.h"
#define WORDS 2000
#define TEXT_SIZE 100000
#define WORDS_FOR_STATE_NUM 100                                             //Комментарии на русском, т.к. читать будут все + Костя
                                                                            //Всё, что можно вынести в функции обозначено FUNCT.

typedef enum{                                                               //Наши состояния
    FOR,                                                                    //Состояние FOR. Распостраняется на for, while, switch
    FOR_BODY,                                                               //Два состояния для отслежки стека циклов
    FOR_SINGLE,
    INIT,                                                                   //Состояние INIT. Распостраняется на int/char/bool/еще добавить
    TYPEDEF,                                                                //Состояние TYPEDEF. Оно почти юзлесс, но убирать его нельзя
    STRUCT,                                                                 //Состояние STRUCT. Не сильно отличается от INIT. (подр. см. 269)
    IF,                                                                     //Состояние IF. Работает точно, как FOR. Но, для отделения от FOR_BODY и FOR_SINGLE после, выделено отдельно.
    CASE,                                                                   //Состояние CASE. Работает пока одно.
    NOTHING,                                                                //Состояние NOTHING. Не пригодилось, но пока оставил.
    EMPTY                                                                   //Начальное состояние.
} state ;

struct Node {
   state value;
 /*char state_Name[20];*/
   struct Node *next;
};                                                          //До основной функции всё, что связано со стеком. Классика.

typedef struct {
    char stateName[WORDS];
    state value;
} stateTypes;

void push(struct Node **s, state value) {
    struct Node *temp = (struct Node *) malloc(sizeof(struct Node));
    temp->value = value;
    temp->next = *s;
    *s = temp;
}

state pop(struct Node **s) {
    /*if (!(*s)) {
        printError("Not enough arguments for calculation");
        return NAN;
    }*/

    state value = (*s)->value;
    struct Node *next = (*s)->next;
    free(*s);
    *s = next;
    return value;
}

state peek(struct Node **s){
    if ((*s))
    return (*s) -> value;
    else return EMPTY;
}

void wordHandler (char *input, int inputSize, char *output, int *outputSize) {

    char word [WORDS] = { 0 };                                              //Буфер для слова
    int wordSize = 0;                                                       //Длина слова в буфере

    int nowSize = WORDS_FOR_STATE_NUM;                                      //now - Массив состояний + их отдельного слова.
    stateTypes now [nowSize];                                               //nowSize - его длин
    int soWeHaveLike = 0;                                                   //soWeHaveLike - число ключ-слов, которые уже нами добавлены, чтобы не бежать по куче пустых.

    struct Node* stateStack = NULL;                                         //Стек состояний
    state lastState = EMPTY;                                                //lastState. Полезный товарищ, хотя и зачастую бесполезный.


    int bracketSequence = 0;                                                //Переменная для верной скобочной последовательности

    now[0].value = FOR;                                                     //Инициализация ключ-слов и подключение их правил
    strcpy (now[0].stateName, "for");                              //FUNCT

    now[1].value = INIT;
    strcpy(now[1].stateName, "int");

    now[2].value = INIT;
    strcpy(now[2].stateName, "char");

    now[3].value = INIT;
    strcpy(now[3].stateName, "bool");

    now[4].value = FOR;
    strcpy(now[4].stateName, "while");

    now[5].value = TYPEDEF;
    strcpy(now[5].stateName, "typedef");

    now[6].value = STRUCT;
    strcpy(now[6].stateName, "struct");

    now[7].value = IF;
    strcpy(now[7].stateName, "if");

    now[8].value = FOR;
    strcpy(now[8].stateName, "switch");

    now[9].value = CASE;
    strcpy(now[9].stateName, "case");

    soWeHaveLike = 10;                                                      //Сейчас их у нас всего 10 :(

    for (int i = 0; i < inputSize; ++i){                                    //Наш цикл

        if (isalnum(input[i]) || input[i] == '_'){                          //Формирование слова

            word[wordSize++] = input[i];

        }
        else{                                                               //При первом же разделителе, смотрим вышло ли слово
            for (int j = 0; j < soWeHaveLike; ++j){                         //Пробегаемся по ключ-словам
                if (!strcmp(word, now[j].stateName)){                       //Нашли ли?                     Вероятно FUNCT.

                    if (peek(&stateStack) == TYPEDEF && (now[j].value == INIT || now[j].value == STRUCT)){  //Если был typedef, смотрим кто же после него
                        pop(&stateStack);                                                                   //int/char/bool или struct
                    }                                                                                       //Но он, наверное, бесползен. Возможно убрать?

                    if (now[j].value == FOR){                                                               //
                        lastState = FOR;
                    }

                    push((&stateStack), now[j].value);                                                      //Пушаем нынешнее состояние в стек

                    if (input [i] != ' '){                                                                  //Если после слова не было пробела, ставим
                        sprintf(output + (*outputSize)++, " ");                             //Ex.: for_(;;)     while_()    if_()
                    }
                    break;                                                                                  //Ну и всё, хватит с меня
                }
            }
            for (int l = 0; l < wordSize; l++)                                                              //Просто операция очистки слова
                word[l] = 0;                                                                                //FUNCT.
            wordSize = 0;
        }

        switch (peek(&stateStack)){

            case IF:                                                                                        //Правила для IF и FOR
            case FOR:

                switch (input[i]) {

                    case ')': {                                                                             //Встретили ')'
                        bracketSequence--;                                                                  //В соответствие с скобочной последовательностью
                        sprintf(output + (*outputSize)++, ")");                             //Напечатали
                        if (!bracketSequence) {                                                            //Если с последовательностью всё хорошо
                            if (input[i] != ' ')                                                           //Проверяем на пробеле ли мы стоим. Нет - ставим
                                sprintf(output + (*outputSize)++, " ");
                            lastState = pop(&stateStack);                                                  //И выходим из области работы с последней скобкой.
                        }
                        break;
                    }

                    case '(': {                                                                             //Встретили '('
                        bracketSequence++;                                                                  //В соответствие с скобочной последовательностью
                        sprintf(output + (*outputSize)++, "(");                             //Напечатали
                        break;
                    }

                    case ';': {                                                                              //Встретили ';'
                        sprintf(output + (*outputSize)++, ";");                              //Напечатали
                        if (input[i + 1] != ' ') {                                                           //Нет дальше пробела?
                            sprintf(output + (*outputSize)++, " ");                          //Печатаем
                        }
                        break;
                    }

                    default:                                                                                //Все оставшиеся символы
                        sprintf(output + (*outputSize)++, "%c", input[i]);                  //Просто печатаем
                        break;

                }

            break;                                                                                          //Тут кончается FOR и IF

            case INIT:                                                                                      //Правила для INIT

                switch (input[i]) {

                    case ';': {                                                                             //Встретили ';'
                        if (output[*outputSize - 1] == ' ')                                                 //Приклеиваем к предыдущему символу
                            (*outputSize)--;                                                                //Для красоты
                                                                                                            //Ex.: int a; char c;
                        sprintf(output + (*outputSize)++, ";", input[i]);                   //Печатаем ';'

                        if (lastState == FOR){                                                              //Если это инициализация в FOR
                            sprintf(output + (*outputSize)++, " ", input[i]);               //То после ; печатаем пробел, а не \n
                        }
                        else sprintf(output + (*outputSize)++, "\n", input[i]);             //Для любой другой инициализации ;\n

                        /*lastState =*/ pop(&stateStack);                                                   //Выносим состояние

                        if (input[i+1] == ' ') i++;                                                         //Строка для избавления ненужного пробела после
                        break;
                    }

                    default: {                                                                              //Все остальные символы - просто печатаем
                        sprintf(output + (*outputSize)++, "%c", input[i]);
                        break;
                    }
                }
            break;                                                                                          //Тут кончается INIT

            /*case NOTHING:{
                sprintf(output, "\n};\n");
                *outputSize += 4;
                lastState = pop(&stateStack);
                break;
            }*/

            /*case STRUCT:{

                switch (input[i]) {
                    case '{': {
                        sprintf(output + (*outputSize)++, "{", input[i]);
                        sprintf(output + (*outputSize)++, "\n", input[i]);
                        if (input[i+1] == ' ') i++;
                        break;
                    }
                    case '}': {
                        sprintf(output + (*outputSize)++, "}", input[i]);
                        pop(&stateStack);
                        push(&stateStack, INIT);
                        break;
                    }
                }
            }*/

            case CASE:                                                                                      //Правила для CASE
                switch (input[i]){

                    case ':': {                                                                             //Встретили ':'
                        //TODO Comment, Space and other skip                            FUNCT               //Нужна функция для пропуска пробелов и комментариев. Сделайте пж, она простая, просто времени нет :(
                        sprintf(output + (*outputSize)++, ":");                             //Печатаем

                        if (input[i+1] == '{'){                                                             //Если у нас case:{
                            sprintf(output+(*outputSize), "{\n");                           //Приклеиваем следующий '{' к ':'
                            (*outputSize) += 2;
                            i++;
                        }
                        else sprintf(output + (*outputSize)++, "\n");                       //Если же просто case: - печатаем \n

                        pop(&stateStack);                                                                   //Выносим состояние
                        break;
                    }

                    default:{
                        sprintf(output + (*outputSize)++, "%c", input[i]);                  //Все остальные символы - просто печатаем
                        break;
                    }

                }
            break;                                                                                          //Тут кончается CASE

            case TYPEDEF:                                                                                   //Правила для TYPEDEF совершенно обычны. Может он юзлесс?

            default:                                                                                        //Стандарт правил

                switch (input[i]){
                    case ';':                                                                               //Для ';', '{', '}'
                    case '{':
                    case '}': {

                        if (peek(&stateStack) == STRUCT && input[i] == '}'){                                //Случай со STRUCT. В нём запрещены пустые {};
                            sprintf(output + (*outputSize)++, "}", input[i]);               //А правила действуют лишь на "конце", причём правила те же, что и у инициализации
                            lastState = pop(&stateStack);                                                   //Поэтому выносим это состояние и заносим INIT
                            push(&stateStack, INIT);                                                //При встрече с }
                            break;                                                                         //Чтобы использовать правила на конце.
                        }                                                                                  //Ex.: struct {                      typedef struct{
                                                                                                            //int a;                or          int biba;
                                                                                                            //} name = { 2 };                   } name;
                        /*if (lastState == EMPTY && input[i] == '{' && stateExisted)
                            push(&stateStack, NOTHING);*/

                        if (output[(*outputSize) - 1] == ' ')                                               //Для красивой "склейки" игнорируем прошлый пробел
                            (*outputSize)--;                                                                //Ведь ставится он обычно при состояниях

                        if (output[(*outputSize) - 2] == '}' && input[i] == ';')                            //Для склейки '}' и ';', т.е. игнора прошлого \n
                            (*outputSize)--;                                                                //Нужна адаптация, т.к. мб комментарий

                        sprintf(output + (*outputSize)++, "%c", input[i]);                  //Просто печать разделителя со \n
                        sprintf(output + (*outputSize)++, "\n", input[i]);

                        if (lastState == FOR && input[i] == '{') {                                          //Для загрузки FOR_BODY
                            push(&stateStack, FOR_BODY);
                        }

                        lastState = EMPTY;                                                                  //Чтобы не загузить FOR_BODY и проч. повторно

                        if (input[i+1] == ' ') i++;                                                         //Для пропуска ненужного пробела после, чтобы не было
                        break;                                                                              //int a; int b; -> int a;
                                                                                                            //                  int b;
                    }
                    default:{                                                                               //Все остальные симолы

                        sprintf(output + (*outputSize)++, "%c", input[i]);                  //Просто печать
                        //TODO Comment, Space and other skip
                        if (lastState == FOR && input[i] == '{') {                                          //Установка FOR_BODY, FOR_SINGLE,
                            push(&stateStack, FOR_BODY);                                            //а также, чтобы не мешал при вложенности IF ставит \n, если у него есть тело
                        }else if (lastState == FOR && input[i] != '{'){
                            sprintf(output + (*outputSize)++, "\n", input[i]);
                            push(&stateStack, FOR_SINGLE);
                        }else if (lastState == IF && input[i] != '{'){
                            sprintf(output + (*outputSize)++, "\n", input[i]);
                        }
                        lastState = EMPTY;                                                                  //Чтобы не устанавливать FOR_BODY/FOR_SINGLE повторно
                        break;

                    }

                }
            break;
        }


    }

}
