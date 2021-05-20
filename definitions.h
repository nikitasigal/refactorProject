#ifndef REFACTORPROJECT_DEFINITIONS_H
#define REFACTORPROJECT_DEFINITIONS_H

#define WORDS 1000                      // Длина слова по умолчанию
#define WORDS_FOR_STATE_NUM 200         // Размер массива состояний
#define TEXT_SIZE 100000                // Размер текста
#define SPECIAL_WORDS_NUMBER 9          // Количество типов данных в Си. unsigned, long, int, char и т.п.

typedef enum {                           //Наши состояния
    FOR,                                 //Состояние FOR. Распостраняется на for, while, switch
    FOR_BODY,                            //Два состояния для отслежки стека циклов
    IF_BODY,
    FOR_SINGLE,
    IF_SINGLE,
    ELSE_BODY,
    ELSE_SINGLE,
    INIT,                                //Состояние INIT для типов данных
    TYPEDEF,                             //Состояние TYPEDEF. Оно почти юзлесс, но убирать его нельзя
    STRUCT,                              //Состояние STRUCT. Не сильно отличается от INIT. (подр. см. 269)
    IF,                                  //Состояние IF. Работает точно, как FOR. Но, для отделения от FOR_BODY и FOR_SINGLE после, выделено отдельно.
    CASE,                                //Состояние CASE. Работает пока одно. (не работает)
    NOTHING,                             //Состояние NOTHING. Для пустых скобок
    EMPTY,                               //Начальное состояние.
    ELSE
} state;

struct Node {                            // Структура стека состояний
    state value;
    struct Node *next;
};

typedef struct {                         // Структура слово - состояние. Обычный массив, в который заносятся слова
    char stateName[WORDS];
    state value;
} stateTypes;

#endif //REFACTORPROJECT_DEFINITIONS_H
