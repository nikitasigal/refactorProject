#ifndef REFACTORPROJECT_STRUCTS_H
#define REFACTORPROJECT_STRUCTS_H

#include "definitions.h"

/*
 * Структура для переменной. Переменная имеет имя и состояние "инициализирована" / "неинициализирована"
 */
typedef struct {
    char value[WORDS];
    bool isInitialized;
} VARIABLE;

/*
 * Перечисление состояний
 */
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
    ELSE,
    FUNC
} state;


// MAP
typedef struct  {
    char key[WORDS];
    int line;
    bool empty;
} Map;

void initElements(Map *m);

void insertElement(Map *m, char *key, int lineNumber);

void checkElement(Map *m, char *key);

void printFooMap(Map *m);

void printVarMap(Map *m);


// STACK
struct Node {                            // Структура стека состояний
    state value;
    struct Node *next;
};

typedef struct {                         // Структура слово - состояние. Обычный массив, в который заносятся слова
    char stateName[WORDS];
    state value;
} stateTypes;

void push(struct Node **s, state value);

state pop(struct Node **s);

state peek(struct Node **s);


// TREE
struct TreeNode {
    char value[WORDS];
    bool isRecursive;
    int childCount;
    struct TreeNode *child[100];
    struct TreeNode *parent;
};

struct TreeNode *
addNode(struct TreeNode *tree, struct TreeNode *curNode, struct TreeNode *nodeParent, char *value, char *parent);


// TREE-STACK
struct stackTreeNode {
    struct TreeNode *tree;
    struct stackTreeNode *next;
};

// FOREST
typedef struct {
    struct stackTreeNode *trees;
    int size;
} Forest;

void pushTree(Forest **forest, struct TreeNode *tree);

#endif //REFACTORPROJECT_STRUCTS_H
