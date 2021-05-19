#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "wordHandler.h"

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

/*struct funcUsed {
    char value[1000];
    struct funcUsed *next;
};

struct funcInitNode {
    char value[1000];
    struct funcUsed *listOfFunc;
    struct funcInitNode *next;
};

void pushFuncInit(struct funcInitNode **s, char *name) {
    struct funcInitNode *temp = (struct funcInitNode *) malloc(sizeof(struct funcInitNode));
    temp->next = *s;
    strcpy(temp->value, name);
    temp->listOfFunc = NULL;
    *s = temp;
}

void pushFuncUsed(struct funcInitNode **s, char *name) {
    struct funcUsed *temp = (struct funcUsed *) malloc(sizeof(struct funcUsed));
    temp->next = (*s)->listOfFunc;
    strcpy(temp->value, name);
    (*s)->listOfFunc = temp;
}

void printFunctions(struct funcInitNode **s) {
    struct funcInitNode *nextInit = (*s);
    while (nextInit) {
        printf("%s:\n", nextInit->value);
        struct funcUsed *nextFunc = (nextInit->listOfFunc);
        while (nextFunc) {
            printf("%s\n", nextFunc->value);
            nextFunc = nextFunc->next;
        }
        printf("\n");
        nextInit = (nextInit)->next;
    }
}*/

struct Node {
    state value;
    struct Node *next;
};                                                          //До основной функции всё, что связано со стеком. Классика.

typedef struct {
    char stateName[WORDS];
    state value;
} stateTypes;

//TODO память не чистится
void push(struct Node **s, state value) {
    struct Node *temp = (struct Node *) malloc(sizeof(struct Node));
    temp->value = value;
    temp->next = *s;
    *s = temp;
}

state pop(struct Node **s) {
    state value = (*s)->value;
    struct Node *next = (*s)->next;
    free(*s);
    *s = next;
    return value;
}

state peek(struct Node **s) {
    if ((*s))
        return (*s)->value;
    else
        return EMPTY;
}

void skip(char *input, char *output, int *i, int *outputSize, int inputSize) {
    //TODO !!! пропуск символа #, ибо в define и include происходит форматирование
    bool isSignificantSymbol = false;
    while (*i < inputSize && !isSignificantSymbol) {
        isSignificantSymbol = true;
        if (input[*i] == '/') {
            if (input[*i + 1] == '/') {
                while (input[*i] != '\n')
                    sprintf(output + (*outputSize)++, "%c", input[(*i)++]);
                continue;
            } else if (input[*i + 1] == '*') {      //TODO есть идея по выравниванию табуляций на каждой строчке комментария. Можно перед /* посчитать кол-во табуляций (пробелов) и рисовать их вручную
                while (input[*i] != '*' || input[*i + 1] != '/')    //TODO upd. CLion превращает \t в пробелы. По умолчанию \t = 4 пробела. Но мы не знаем, сколько на самом деле пользователь поставил, так что не пойдёт
                    sprintf(output + (*outputSize)++, "%c", input[(*i)++]);
                sprintf(output + *outputSize, "*/");
                *outputSize += 2;
                *i += 2;
                continue;
            }
        }
        if (input[*i] == '\'') {
            sprintf(output + (*outputSize)++, "'");
            (*i)++;
            while ((input[*i - 1] == '\\' && input[*i - 2] != '\\') || input[*i] != '\'')
                sprintf(output + (*outputSize)++, "%c", input[(*i)++]);
            continue;
        }
        if (input[*i] == '\"') {
            sprintf(output + (*outputSize)++, "\"");
            (*i)++;
            while ((input[*i - 1] == '\\' && input[*i - 2] != '\\')|| input[*i] != '"')
                sprintf(output + (*outputSize)++, "%c", input[(*i)++]);
            continue;
        }
    }
}

int nesting(struct Node **s) {
    int res = 0;
    struct Node *next = (*s);
    while (next) {
        if (next->value == FOR || next->value == FOR_BODY || next->value == FOR_SINGLE) {
            res++;
        }
        next = (next)->next;
    }
    return res;
}

void printTabs(int tabCount, char *output, int *outputSize) {           // Рисуем табуляции
    for (int i = 0; i < tabCount; ++i) {
        sprintf(output + (*outputSize)++, "\t");
    }
}

bool isElse(char *input, int i) {
    if (!strncmp(&input[i + 1], "else", 4 * sizeof (char)) || !strncmp(&input[i + 2], "else", 4))
        return true;
    return false;
}

void wordHandler(char *input, int inputSize, char *output, int *outputSize) {
    char word[WORDS] = {0};                                                 //Буфер для слова
    int wordSize = 0;                                                       //Длина слова в буфере
    //now - Массив состояний + их отдельного слова.
    stateTypes now[WORDS_FOR_STATE_NUM] = {{"while",   FOR},
                                           {"for",     FOR},
                                           {"switch",  IF},
                                           {"if",      IF},
                                           {"int",     INIT},
                                           {"char",    INIT},
                                           {"double",  INIT},
                                           {"float",   INIT},
                                           {"short",   INIT},
                                           {"long",    INIT},
                                           {"bool",    INIT},
                                           {"void",    INIT},
                                           {"typedef", TYPEDEF},
                                           {"struct",  STRUCT},
                                           {"enum",    STRUCT},
                                           {"case",    CASE},
                                           {"default", CASE},
                                           {"else",    ELSE},
                                           {"state", INIT}};

    struct Node *stateStack = NULL;                                            //Стек состояний
    state lastState = EMPTY;                                                   //lastState. Полезный товарищ, хотя и зачастую бесполезный

    //
    /*struct funcInitNode *functions = NULL;
    pushFuncInit(&functions, "firstFoo");
    pushFuncUsed(&functions, "testFunc");
    pushFuncUsed(&functions, "secondTest");

    pushFuncInit(&functions, "function");
    pushFuncInit(&functions, "init");
    pushFuncInit(&functions, "biba");

    printFunctions(&functions);*/
    //

    bool isEqu = false;
    bool isFunc = false;

    int bracketSequence = 0;                                                   //Переменная для верной скобочной последовательности
    int tabsCount = 0;                                                         // Храним табуляции если чо
    int nestResult = 0;                                                        // Хранилище вложенности циклов

    for (int i = 0; i < inputSize; ++i) {                                      //Наш цикл
        if (nesting(&stateStack) > nestResult) {                               // Проверяем сколько циклов у нас лежит в стеке состояний.
            nestResult = nesting(&stateStack);                                 // TODO это неэффективно. Нам на КАЖДОМ символе надо бегать по стеку (причём два раза)
        }
        //skip(input, output, &i, outputSize, inputSize);
        if (isalnum(input[i]) || input[i] == '_') {                            //Формирование слова
            word[wordSize++] = input[i];
        } else {                                                               //При первом же разделителе, смотрим вышло ли слово
            for (int j = 0; j < WORDS_FOR_STATE_NUM; ++j) {                    //Пробегаемся по ключ-словам
                if (!strcmp(word, now[j].stateName)) {                         //Нашли ли?               Вероятно FUNCT.
                    if (peek(&stateStack) == TYPEDEF && (now[j].value == INIT || now[j].value == STRUCT)) {  //Если был typedef, смотрим кто же после него
                       pop(&stateStack);                                                                    //int/char/bool или struct
                       push(&stateStack, now[j].value);                                                    //Но он, наверное, бесползен. Возможно убрать?
                    } else if (now[j].value == STRUCT) {
                        push(&stateStack, INIT);
                        lastState = STRUCT;
                    } else {
                        push((&stateStack), now[j].value);                                   //Пушаем нынешнее состояние в стек
                    }

                    /*if (now[j].value == FOR) {
                        lastState = FOR;
                    }*/


                    if (peek(&stateStack) != INIT) {
                        lastState = peek(&stateStack);
                    }

                    if (input[i] != ' ' && lastState != CASE) {                          //Если после слова не было пробела, ставим
                        sprintf(output + (*outputSize)++, " ");            //Ex.: for_(;;)  while_()  if_()
                    }
                    break;                                                               //Ну и всё, хватит с меня
                }
            }
            for (int l = 0; l < wordSize; l++)                                           //Просто операция очистки слова
                word[l] = 0;                                                             //FUNCT.
            wordSize = 0;
        }

        switch (peek(&stateStack)) {
            case IF:                                                                          //Правила для IF и FOR
            case FOR:
                switch (input[i]) {
                    case ')': {                                                               //Встретили ')'
                        bracketSequence--;                                                    //В соответствие с скобочной последовательностью
                        sprintf(output + (*outputSize)++, ")");                 //Напечатали

                        if (!bracketSequence) {                                               //Если с последовательностью всё хорошо
                            //TODO Вопрос: мы сейчас стоим на input[i] = ')'. Вы уверены, что хотите проверить это ещё раз?
                            //TODO Ответ: была ошибка, смотреть надо следующий символ. Но возможно здесь нужен скип комента
                            if (input[i + 1] != ' ')                                            //Проверяем на пробеле ли мы стоим. Нет - ставим
                                sprintf(output + (*outputSize)++, " ");
                            lastState = pop(&stateStack);                                     //И выходим из области работы с последней скобкой.
                        }
                        break;
                    }

                    case '(': {                                                               //Встретили '('
                        bracketSequence++;                                                    //В соответствие с скобочной последовательностью
                        sprintf(output + (*outputSize)++, "(");                 //Напечатали
                        break;
                    }

                    case ';': {                                                               //Встретили ';'
                        sprintf(output + (*outputSize)++, ";");                 //Напечатали
                        if (input[i + 1] != ' ') {                                            //Нет дальше пробела?
                            sprintf(output + (*outputSize)++, " ");             //Печатаем
                        }
                        break;
                    }

                    default:                                                                  //Все оставшиеся символы
                        skip(input, output, &i, outputSize, inputSize);                       // Пропустим все комментарии и " ", чтобы не посчитать случайно ( )
                        sprintf(output + (*outputSize)++, "%c", input[i]);      //Просто печатаем
                        if (input[i] == '\n')
                            printTabs(tabsCount, output, outputSize);
                        break;
                }

                break;                                                                    //Тут кончается FOR и IF

            case INIT:                                                                    //Правила для INIT
                switch (input[i]) {
                    case ';': {                                                           //Встретили ';'
                        if (output[*outputSize - 1] == ' ')                               //Приклеиваем к предыдущему символу
                            (*outputSize)--;                                              //Для красоты
                        //Ex.: int a; char c;
                        sprintf(output + (*outputSize)++, ";");             //Печатаем ';'

                        while (peek(&stateStack) == INIT)                                 // Клинический случай: long long int a; Тут три инициализация типа, поэтому
                            pop(&stateStack);                                             // убиваем ВСЕ INIT

                        while ((peek(&stateStack) == FOR_SINGLE || peek(&stateStack) == IF_SINGLE)) { // Если после INIT мы вернулись в одиночные for / if, то они
                            pop(&stateStack);                                                       // автоматически закончились, выгружаем их
                            tabsCount--;
                            if (isElse(input, i))
                                break;
                        }

                        if (peek(&stateStack) == ELSE_SINGLE) {
                            pop(&stateStack);
                            tabsCount--;
                        }

                        if (lastState == FOR) {                                           //Если это инициализация в FOR
                            sprintf(output + (*outputSize)++, " ");         //То после ; печатаем пробел, а не \n
                        } else {
                            sprintf(output + (*outputSize)++, "\n");
                            printTabs(tabsCount, output, outputSize);
                        }                                                                 //Для любой другой инициализации ;\n

                        /*while (peek(&stateStack) == INIT) {                             // TODO: выгрузка всех INIT. Надо бы сделать где-то
                            pop(&stateStack);                                             // TODO: сейчас сделаю отделение на функцию
                        }*/
                        /*lastState =*/ /*pop(&stateStack);  */                               //Выносим состояние


                        isEqu = false;

                        if (input[i + 1] == ' ')
                            i++;                                                          //Строка для избавления ненужного пробела после
                        break;
                    }

                    case '=':
                        isEqu = true;
                        sprintf(output + (*outputSize)++, "=");
                        break;

                    case '{':
                        sprintf(output + (*outputSize)++, "{");
                        if (!isEqu && lastState == STRUCT){
                            pop(&stateStack);
                            push(&stateStack, STRUCT);
                            sprintf(output + (*outputSize)++, "\n");
                            if (input[i + 1] == ' ')
                                i++;
                            tabsCount++;
                            printTabs(tabsCount, output, outputSize);
                        }
                        break;

                    case '(':
                        if (!isEqu)
                            isFunc = true;
                        sprintf(output + (*outputSize)++, "(");
                        break;

                    case ',':
                        if (isFunc) {
                            sprintf(output + (*outputSize)++, ",");
                            sprintf(output + (*outputSize)++, " ");
                            if (input[i + 1] == ' ')
                                i++;
                            pop(&stateStack);
                            break;
                        } else {
                            sprintf(output + (*outputSize)++, ",");
                        }
                        break;
                    case ')':
                        if (isFunc){
                            sprintf(output + (*outputSize)++, ")");
                            sprintf(output + (*outputSize)++, " ");
                            if (input[i + 1] == ' ')
                                i++;
                            while (peek(&stateStack) == INIT)                     // Тут проблема была в том, что не все INIT выносились, поэтому я поставил while
                                pop(&stateStack);                                 //Выносим состояние
                            isEqu = false;
                            isFunc = false;
                            break;
                        }
                    default: {                                                            //Все остальные символы - просто печатаем
                        sprintf(output + (*outputSize)++, "%c", input[i]);
                        break;
                    }
                }
                break;                                                                    //Тут кончается INIT

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

            case CASE: //TODO !!!!! Может быть дополнительно понадобится состояние switch. Каждый case должен уменьшать табуляцию. На примерах можно увидеть  //Правила для CASE
                switch (input[i]) {
                    case ':': {                                                                  //Встретили ':'
                        tabsCount++;
                        //TODO Comment, Space and other skip   FUNCT

                        if (input[i - 1] == ' ')                                                 // Убираем лишний пробел между case и ':'
                            outputSize--;

                        skip(input, output, &i, outputSize, inputSize);                          // Скипаем комментарии и т.п.
                        sprintf(output + (*outputSize)++, ":");                    //Печатаем

                        if (input[i + 1] == '{') {
                            sprintf(output + (*outputSize), " {\n");
                            (*outputSize) += 3;
                            i += 1;
                        } else if (input[i + 2] == '{') {                                         //Если у нас case:{
                            sprintf(output + (*outputSize), "{\n");                 //Приклеиваем следующий '{' к ':'
                            (*outputSize) += 2;
                            i += 2;
                        } else
                            sprintf(output + (*outputSize)++, "\n");                //Если же просто case: - печатаем \n и табуляции

                        if (input[i + 1] == ' ')                                                  // Убираем лишний пробел после ':', если он был
                            ++i;

                        printTabs(tabsCount, output, outputSize);

                        pop(&stateStack);                                                         //Выносим состояние

                        break;
                    }

                    default: {
                        sprintf(output + (*outputSize)++, "%c", input[i]);         //Все остальные символы - просто печатаем
                        break;
                    }

                }
                break;                                                                           //Тут кончается CASE
            case ELSE:                                                                           // else - тот же if, но без скобок (). Он может быть single или простой
                pop(&stateStack);                                                                // Поэтому помечаем его if'ом, а в default с ним разберутся
            case TYPEDEF:                                                                        //Правила для TYPEDEF совершенно обычны. Может он юзлесс?
            default:                                                                             //Стандарт правил
                switch (input[i]) {
                    case ';':                                                                    //Для ';', '{', '}'
                    case '{':
                    case '}': {
                        if (input[i] == '{')                                                     // Увеличиваем или уменьшаем табуляцию
                            tabsCount++;                                                         // в зависимости от скобки
                        else if (input[i] == '}')
                            tabsCount--;

                        if (peek(&stateStack) == STRUCT && input[i] == '}') {                    //Случай со STRUCT. В нём запрещены пустые {};
                            (*outputSize)--;
                            sprintf(output + (*outputSize)++, "}");                //А правила действуют лишь на "конце", причём правила те же, что и у инициализации
                            lastState = pop(&stateStack);                                        //Поэтому выносим это состояние и заносим INIT
                            push(&stateStack, INIT);                                       //При встрече с }
                            break;                                                               //Чтобы использовать правила на конце.
                        }                                                                        //Ex.: struct {                      typedef struct{
                                                                                                 //int a;                or          int biba;
                                                                                                 //} value = { 2 };                   } value;
                      /*if (lastState == EMPTY && input[i] == '{' && stateExisted)
                         push(&stateStack, NOTHING);*/

                        if (peek(&stateStack) == ELSE_SINGLE && input[i] == ';') {
                            pop(&stateStack);
                            tabsCount--;
                        }

                        if ((peek(&stateStack) == NOTHING || peek(&stateStack) == ELSE_BODY || peek(&stateStack) == FOR_BODY || peek(&stateStack) == IF_BODY) && input[i] == '}'){    // Закончился for или if или пустые скобки?
                            pop(&stateStack);
                            (*outputSize)--;
                        }

                        if (lastState != FOR && lastState != IF && lastState != ELSE && peek(&stateStack) != STRUCT && input[i] == '{') {       // Если мы внезапно встретили скобки, но не после for или if, то скажем,
                            push(&stateStack, NOTHING);                               // что это пустые/незначащие скобки (NOTHING)
                        }

                        if (output[(*outputSize) - 1] == ' ')                               //Для красивой "склейки" игнорируем прошлый пробел
                            (*outputSize)--;                                                //Ведь ставится он обычно при состояниях

                        if (output[(*outputSize) - 2] == '}' && input[i] == ';')            //Для склейки '}' и ';', т.е. игнора прошлого \n
                            (*outputSize)--;                                                //Нужна адаптация, т.к. мб комментарий

                        /*if (peek(&stateStack) == NOTHING && input[i] == '}')
                            lastState = pop(&stateStack);*/

                        if (lastState == FOR && input[i] == '{')                             //Для загрузки FOR_BODY
                            push(&stateStack, FOR_BODY);

                        if (lastState == IF && input[i] == '{')                              // Для загрузки IF_BODY
                            push(&stateStack, IF_BODY);

                        if (lastState == ELSE && input[i] == '{')                             //Для загрузки FOR_BODY
                            push(&stateStack, ELSE_BODY);

                        while ((peek(&stateStack) == FOR_SINGLE || peek(&stateStack) == IF_SINGLE) && !isElse(input, i)) {   // Если мы что-то выгрузили,
                            pop(&stateStack);                                                         // то все SINGLE выше закончились, выгружаем их
                            tabsCount--;
                        }

                        sprintf(output + (*outputSize), "%c\n", input[i]);      //Просто печать разделителя со \n и табуляций
                        *outputSize += 2;
                        printTabs(tabsCount, output, outputSize);

                        lastState = EMPTY;                                                    //Чтобы не загузить FOR_BODY и проч. повторно

                        if (input[i + 1] == ' ')
                            i++;                                                              //Для пропуска ненужного пробела после, чтобы не было
                        break;                                                                //int a; int b; -> int a;
                                                                                              //                 int b;
                    }
                    default: {
                        //Все остальные символы
                        //TODO Comment, Space and other skip
                        skip(input, output, &i, outputSize, inputSize);
                        if (lastState == FOR && (input[i] == '{' || input[i + 1] == '{')) {                            //Установка FOR_BODY, FOR_SINGLE, IF_BODY, IF_SINGLE
                            push(&stateStack, FOR_BODY);                                //а также, чтобы не мешал при вложенности IF ставит \n, если у него есть тело
                            ++tabsCount;
                            printTabs(tabsCount, output, outputSize);
                            lastState = EMPTY;
                        } else if (lastState == IF && (input[i] == '{' || input[i + 1] == '{')) {
                            push(&stateStack, IF_BODY);                                //а также, чтобы не мешал при вложенности IF ставит \n, если у него есть тело
                            ++tabsCount;
                            printTabs(tabsCount, output, outputSize);
                            lastState = EMPTY;
                        } else if (lastState == FOR && (input[i] != '{' && input[i + 1] != '{')) {
                            push(&stateStack, FOR_SINGLE);
                            ++tabsCount;
                            if (input[i] != '\n') {                                         // После комментариев в specialSymbols ставится \n. Поэтому после for может быть два \n. Исправляем это
                                sprintf(output + (*outputSize)++, "\n");
                                printTabs(tabsCount, output, outputSize);
                            }
                            lastState = EMPTY;
                        } else if (lastState == IF && (input[i] != '{' && input[i + 1] != '{')) {
                            push(&stateStack, IF_SINGLE);
                            ++tabsCount;
                            if (input[i] != '\n') {                                         // После комментариев в specialSymbols ставится \n. Поэтому после for может быть два \n. Исправляем это
                                sprintf(output + (*outputSize)++, "\n");
                                printTabs(tabsCount, output, outputSize);
                            }
                            lastState = EMPTY;
                        } else if (lastState == ELSE && ((input[i] == '{' && input[i + 1] == '{'))) {
                            push(&stateStack, ELSE_BODY);                                //а также, чтобы не мешал при вложенности IF ставит \n, если у него есть тело
                            ++tabsCount;
                            //printTabs(tabsCount, output, outputSize);
                            lastState = EMPTY;
                        } else if (lastState == ELSE && ((input[i] != '{' && input[i + 1] != '{'))) {
                            push(&stateStack, ELSE_SINGLE);                                //а также, чтобы не мешал при вложенности IF ставит \n, если у него есть тело
                            ++tabsCount;
                            if (input[i] != '\n') {                                         // После комментариев в specialSymbols ставится \n. Поэтому после for может быть два \n. Исправляем это
                                sprintf(output + (*outputSize)++, "\n");
                                printTabs(tabsCount, output, outputSize);
                            }
                            lastState = EMPTY;
                        }


                        if (input[i] != ' ' || lastState != ELSE) {                    // Костыль для else, не знаю как иначе. Если после else идёт пробел, при этом нет {}, то пробел печатался
                            sprintf(output + (*outputSize)++, "%c", input[i]);
                        }

                        if (input[i] == '\n')
                            printTabs(tabsCount, output, outputSize);

                        break;
                    }
                }
                break;
        }
    }
}
