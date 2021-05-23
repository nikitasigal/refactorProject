#include "checkRecursion.h"

// skip версии 3.0. Скипает ненужные знаки, не печатая их, как делает это skip 1.0
void skip4(const char *input, int *i, int inputSize, int *lineNumber) {
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
            while ((input[*i - 1] == '\\' && input[*i - 2] != '\\') || input[*i] != '\'')
                (*i)++;
            (*i)++;
            continue;
        }
        if (input[*i] == '"') {
            (*i)++;
            while (input[*i - 1] == '\\' || input[*i] != '"')
                (*i)++;
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

void skipComments2(const char *input, int inputSize, int *i, int *lineNumber) {
    if (input[(*i)] == '/' && input[(*i) + 1] == '/' || input[(*i)] == '/' && input[(*i)] == '*' || input[*i] == '"' || input[*i] == '\'') {
        skip4(input, i, inputSize, lineNumber);
        --(*i);
    }
}

// FOREST
void getInit(Forest *forest, char *input, int inputSize, stateTypes *now, int nowSize) {
    char word[WORDS] = {0};
    int wordSize = 0;

    for (int i = 0; i < inputSize; ++i) {
        if (isalnum(input[i]) || input[i] == '_') {
            word[wordSize++] = input[i];
        } else {
            skipComments2(input, inputSize, &i, NULL);
            for (int j = 0; j < nowSize && strlen(word) != 0; ++j) {
                if (!strcmp(now[j].stateName, word) && now[j].value == INIT) {
                    // Мы прочитали тип данных: void, int ...
                    skip4(input, &i, inputSize, NULL);
                    clearWord(word, &wordSize);

                    // Теперь прочитаем следующее слово, считая, что дальше пойдёт имя переменной / функции
                    readWord(input, word, &wordSize, &i);

                    // Пустое слово - не инициализация вовсе
                    if (strlen(word) == 0)
                        break;

                    // Прочитали слово. Далее, смотрим, встретили ли мы '('. Если да, то это функция, добавляем её в лес
                    skip4(input, &i, inputSize, NULL);
                    if (input[i] == '(') {
                        struct TreeNode *tree = NULL;
                        tree = addNode(NULL, tree, NULL, word, NULL);
                        pushTree(&forest, tree);
                    }
                }
            }
            clearWord(word, &wordSize);
        }
    }
}

void getUsed(Forest *forest, char *input, int inputSize, stateTypes *now, int nowSize) {
    char word[WORDS] = {0};
    int wordSize = 0;

    // Функция, в которой мы находимся в данный момент
    char currentFoo[WORDS] = {0};

    int bracketSequence = 0;

    for (int i = 0; i < inputSize; ++i) {
        if (isalnum(input[i]) || input[i] == '_') {
            word[wordSize++] = input[i];
        } else {
            // Если мы встретили { или }, находясь в функции, то изменяем bracketSequence
            if (input[i] == '{')
                bracketSequence++;
            if (input[i] == '}')
                bracketSequence--;
if (!strcmp(word, "getInit")) {
    printf("asd");
}
            // Если bracketSequence == 0, сбрасываем currentFoo
            if (bracketSequence == 0) {
                int tempSize = (int)strlen(currentFoo);
                clearWord(currentFoo, &tempSize);
            }
            
            skipComments2(input, inputSize, &i, NULL);
            for (int j = 0; j < nowSize && strlen(word) != 0; ++j) {
                if (!strcmp(now[j].stateName, word) && now[j].value == INIT) {
                    // Мы прочитали тип данных: void, int ...
                    skip4(input, &i, inputSize, NULL);
                    clearWord(word, &wordSize);

                    // Теперь прочитаем следующее слово, считая, что дальше пойдёт имя переменной / функции
                    readWord(input, word, &wordSize, &i);

                    // Пустое слово - не инициализация вовсе
                    if (strlen(word) == 0)
                        break;

                    // Прочитали слово. Далее, смотрим, встретили ли мы '('. Если да, то это функция, мы в ней
                    skip4(input, &i, inputSize, NULL);
                    if (input[i] == '(') {
                        strcpy(currentFoo, word);
                        // Пропустим ()
                        while (input[i] != ';' && input[i] != '{')
                            i++;
                        i--;
                    }
                    
                    // Обработали, продолжаем
                    clearWord(word, &wordSize);
                    continue;
                }
            }
            // Если это не инициализация, то, возможно, вызов функции
            struct stackTreeNode *foundWordTree = forest->trees;
            bool added = false;
            for (int j = 0; j < forest->size && strlen(word) != 0; ++j) {
                if (!strcmp(word, foundWordTree->tree->value)) {
                    // Да, это вызов функции
                    // Давайте найдём дерево функции, в которой мы сейчас находимся
                    struct stackTreeNode *currentFooTree = forest->trees;
                    for (int k = 0; k < forest->size; ++k) {
                        if (!strcmp(currentFoo, currentFooTree->tree->value)) {
                            // Добавляем в найденное дерево дитя
                            addNode(currentFooTree->tree, currentFooTree->tree, NULL, foundWordTree->tree->value,
                                    currentFooTree->tree->value);
                            added = true;
                            break;
                        } else {
                            // Ищем дальше
                            currentFooTree = currentFooTree->next;
                        }
                    }
                } else {
                    // Ищем дальше
                    foundWordTree = foundWordTree->next;
                }
                if (added)
                    break;
            }
            
            clearWord(word, &wordSize);
        }
    }
}

void buildTree(Forest *forest, struct TreeNode *curTree, struct TreeNode *child) {
    // Достроим дерево, за исключением деревьев, уже учавствующих в рекурсии
    struct stackTreeNode *tempTree = forest->trees;
    while (tempTree != NULL) {
        // Мы нашли эту функцию (дитя) в списке
        if (!strcmp(tempTree->tree->value, child->value)) {
            // Рекурсивная функция уже - не берём
            if (tempTree->tree->isRecursive)
                break;

            // Переберём детей найденного дерева
            for (int i = 0; i < tempTree->tree->childCount; ++i) {
                addNode(curTree, child, NULL, tempTree->tree->child[i]->value, child->value);
                buildTree(forest, curTree, tempTree->tree->child[i]);
            }
        }
        tempTree = tempTree->next;
    }
}

void checkRecursion(char *input, int inputSize, stateTypes *now, int nowSize) {
    Forest *forest = (Forest *) malloc(sizeof (Forest));
    forest->size = 0;
    forest->trees = NULL;
    getInit(forest, input, inputSize, now, nowSize);
    getUsed(forest, input, inputSize, now, nowSize);

    struct stackTreeNode *tempTree = forest->trees;
    while (tempTree != NULL) {
        // Переберём все вершины из второго уровня дерева и достроим их
        for (int i = 0; i < 1/*tempTree->tree->childCount*/; ++i) {
            buildTree(forest, tempTree->tree, tempTree->tree->child[i]);
        }
        tempTree = tempTree->next;
    }
}
