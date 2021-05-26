#include "wordHandler.h"

void skip(char *input, char *output, int *i, int *outputSize, int inputSize) {
    bool isSignificantSymbol = false;
    while (*i < inputSize && !isSignificantSymbol) {
        isSignificantSymbol = true;

        // Пропуск комментариев
        if (input[*i] == '/') {
            if (input[*i + 1] == '/') {
                while (input[*i] != '\n')
                    sprintf(output + (*outputSize)++, "%c", input[(*i)++]);
                continue;
            } else if (input[*i + 1] == '*') {
                while (input[*i] != '*' || input[*i + 1] != '/')
                    sprintf(output + (*outputSize)++, "%c", input[(*i)++]);
                sprintf(output + *outputSize, "*/");
                *outputSize += 2;
                *i += 2;
                continue;
            }
        }

        // Пропуск ' '
        if (input[*i] == '\'') {
            sprintf(output + (*outputSize)++, "'");
            (*i)++;
            while ((input[*i - 1] == '\\' && input[*i - 2] != '\\') || input[*i] != '\'')
                sprintf(output + (*outputSize)++, "%c", input[(*i)++]);
            continue;
        }

        // Пропуск " "
        if (input[*i] == '\"') {
            sprintf(output + (*outputSize)++, "\"");
            (*i)++;
            while ((input[*i - 1] == '\\' && input[*i - 2] != '\\')|| input[*i] != '"')
                sprintf(output + (*outputSize)++, "%c", input[(*i)++]);
            continue;
        }

        // Пропуск # (чтобы define не форматировался
        if (input[*i] == '#') {
            sprintf(output + (*outputSize)++, "#");
            (*i)++;
            while (input[*i] != '\n')
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

void
wordHandler(char *input, int inputSize, char *output, int *outputSize, stateTypes *now, int nowSize, int *nestingArray,
            int *nestingSize) {
    char word[WORD_LENGTH] = {0};                                              //Буфер для слова
    int wordSize = 0;                                                          //Длина слова в буфере

    struct Node *stateStack = NULL;                                            //Стек состояний
    state lastState = EMPTY;                                                   //lastState. Полезный товарищ, хотя и зачастую бесполезный

    bool isEqu = false;
    bool isFunc = false;

    int bracketSequence = 0;                                                   //Переменная для верной скобочной последовательности
    int tabsCount = 0;                                                         // Храним табуляции если чо
    int nestResult = 0;                                                        // Хранилище вложенности циклов

    for (int i = 0; i < inputSize; ++i) {                                      //Наш цикл
        if (nesting(&stateStack) > nestResult) {                               // Проверяем сколько циклов у нас лежит в стеке состояний.
            nestResult = nesting(&stateStack);
        }

        if (isalnum(input[i]) || input[i] == '_') {                            //Формирование слова
            word[wordSize++] = input[i];
        } else if (strlen(word) != 0) {                                        //При первом же разделителе, смотрим вышло ли слово
            for (int j = 0; j < nowSize; ++j) {                                //Пробегаемся по ключ-словам
                if (!strcmp(word, now[j].stateName)) {                         //Нашли ли?
                    if (peek(&stateStack) == TYPEDEF && (now[j].value == INIT || now[j].value == STRUCT)) {  //Если был typedef, смотрим кто же после него
                       pop(&stateStack);                                                                     //int/char/bool или struct
                       push(&stateStack, now[j].value);                                                      //Но он, наверное, бесползен. Возможно убрать?
                    } else if (now[j].value == STRUCT) {
                        push(&stateStack, INIT);
                        if (lastState != FOR)
                            lastState = STRUCT;
                    } else {
                        skip(input, output, &i, outputSize, inputSize);
                        if (now[j].value != INIT || !(input[i] == ')' || input[i + 1] == ')'))          // Проверка, преобразование типов это или нет
                            push((&stateStack), now[j].value);                                          //Пушаем нынешнее состояние в стек
                    }

                    if (peek(&stateStack) != INIT) {
                        lastState = peek(&stateStack);
                    }

                    if ((input[i] != ' ' && lastState != CASE) && input[i] != ')') {     //Если после слова не было пробела, ставим
                        sprintf(output + (*outputSize)++, " ");            //Ex.: for_(;;)  while_()  if_()
                    }
                    break;                                                               //Ну и всё, хватит с меня
                }
            }

            clearWord(word, &wordSize);
        }

        switch (peek(&stateStack)) {
            case IF:                                                                          //Правила для IF и FOR
            case FOR:
                switch (input[i]) {
                    case ')': {                                                               //Встретили ')'
                        bracketSequence--;                                                    //В соответствие с скобочной последовательностью
                        sprintf(output + (*outputSize)++, ")");                 //Напечатали

                        if (!bracketSequence) {                                               //Если с последовательностью всё хорошо
                            skip(input, output, &i, outputSize, inputSize);
                            if (input[i + 1] != ' ')                                          //Проверяем на пробеле ли мы стоим. Нет - ставим
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

                        while (peek(&stateStack) == INIT)                                   // Клинический случай: long long int a; Тут три инициализация типа, поэтому
                            pop(&stateStack);                                               // убиваем ВСЕ INIT

                        while (peek(&stateStack) == FOR_SINGLE || peek(&stateStack) == IF_SINGLE) { // Если после INIT мы вернулись в одиночные for / if, то они
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

                        isEqu = false;

                        if (input[i + 1] == ' ')
                            i++;                                                          //Строка для избавления ненужного пробела после
                        break;
                    }

                    case '=': {
                        isEqu = true;
                        sprintf(output + (*outputSize)++, "=");
                        break;
                    }

                    case '{': {
                        sprintf(output + (*outputSize)++, "{");
                        if (!isEqu && lastState == STRUCT) {
                            pop(&stateStack);
                            push(&stateStack, STRUCT);
                            sprintf(output + (*outputSize)++, "\n");
                            if (input[i + 1] == ' ')
                                i++;
                            tabsCount++;
                            printTabs(tabsCount, output, outputSize);
                        }
                        break;
                    }

                    case '(': {
                        if (!isEqu)
                            isFunc = true;
                        sprintf(output + (*outputSize)++, "(");
                        break;
                    }

                    case ',': {
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
                    }

                    case ')': {
                        if (isFunc) {
                            sprintf(output + (*outputSize)++, ")");
                            sprintf(output + (*outputSize)++, " ");
                            if (input[i + 1] == ' ')
                                i++;
                            while (peek(&stateStack) ==
                                   INIT)                     // Тут проблема была в том, что не все INIT выносились, поэтому я поставил while
                                pop(&stateStack);                                 //Выносим состояние
                            isEqu = false;
                            isFunc = false;
                            break;
                        }
                    }
                    default: {                                                            //Все остальные символы - просто печатаем
                        skip(input, output, &i, outputSize, inputSize);
                        sprintf(output + (*outputSize)++, "%c", input[i]);
                        break;
                    }
                }
                break;                                                                    //Тут кончается INIT
            case CASE:
                switch (input[i]) {
                    case ':': {                                                                  //Встретили ':'
                        tabsCount++;

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
                        skip(input, output, &i, outputSize, inputSize);
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

                        if ((peek(&stateStack) == NOTHING || peek(&stateStack) == ELSE_BODY ||
                            peek(&stateStack) == FOR_BODY || peek(&stateStack) == IF_BODY) && input[i] == '}') {    // Закончился for или if или пустые скобки?
                            pop(&stateStack);
                            (*outputSize)--;
                        }

                        if (lastState != FOR && lastState != IF && lastState != ELSE && peek(&stateStack) != STRUCT && input[i] == '{') {       // Если мы внезапно встретили скобки, но не после for или if, то скажем,
                            push(&stateStack, NOTHING);                               // что это пустые/незначащие скобки (NOTHING)
                        }

                        if (output[(*outputSize) - 1] == ' ' && input[i] != '{')                               //Для красивой "склейки" игнорируем прошлый пробел
                            (*outputSize)--;                                                //Ведь ставится он обычно при состояниях

                        if (output[(*outputSize) - 2] == '}' && input[i] == ';')            //Для склейки '}' и ';', т.е. игнора прошлого \n
                            (*outputSize)--;                                                //Нужна адаптация, т.к. мб комментарий

                        if (lastState == FOR && input[i] == '{')                             //Для загрузки FOR_BODY
                            push(&stateStack, FOR_BODY);

                        if (lastState == IF && input[i] == '{')                              // Для загрузки IF_BODY
                            push(&stateStack, IF_BODY);

                        if (lastState == ELSE && input[i] == '{')                             //Для загрузки FOR_BODY
                            push(&stateStack, ELSE_BODY);

                        while ((peek(&stateStack) == FOR_SINGLE || peek(&stateStack) == IF_SINGLE || peek(&stateStack) == ELSE_SINGLE)) {   // Если мы что-то выгрузили,
                            pop(&stateStack);                                                         // то все SINGLE выше закончились, выгружаем их
                            tabsCount--;
                            if (isElse(input, i))
                                break;
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


                        if (input[i] != ' ' || (input[i] == ' ' && output[*outputSize - 1] != '\t')) {      // Костыль для else, не знаю как иначе. Если после else идёт пробел, при этом нет {}, то пробел печатался
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

    // Запоминаем nesting
    nestingArray[(*nestingSize)++] = nestResult;
}
