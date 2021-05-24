#include "checkRecursion.h"

/*
 * Заполняет массив рекурсивных цепочек. Проверяет, была ли данная цепочка в массиве. Если нет - добавляет
 */
void fillChainsArray() {
    bool was = false;
    for (int i = 0; i < chains.size; ++i) {
        if (tempChain.size != chains.value[i].size)
            continue;

        bool used = true;
        for (int j = 0; j < tempChain.size; ++j) {
            if (strcmp(tempChain.value[j], chains.value[i].value[j]) != 0) {
                used = false;
                break;
            }
        }

        if (used) {
            was = true;
            break;
        }
    }

    if (!was) {
        for (int i = 0; i < tempChain.size; ++i) {
            strcpy(chains.value[chains.size].value[i], tempChain.value[i]);
            chains.value[chains.size].size++;
        }
        chains.size++;
    }

    tempChain.size = 0;
}

/*
 * Выводит рекурсивные цепочки
 */
void printChains() {
    for (int i = 0; i < chains.size; ++i) {
        printf("Recursion chain: ");
        for (int j = chains.value[i].size - 1; j >= 0; --j) {
            printf("%s - ", chains.value[i].value[j]);
        }
        printf("%s\n", chains.value[i].value[chains.value[i].size - 1]);
    }
}

/*
 * Ищет первое попавшееся вхождение value в дерево, не помеченное маркой isUsed
 */
struct TreeNode *find(struct TreeNode *tree, char *value) {
    struct TreeNode *temp;
    if (!strcmp(tree->value, value)) {
        if (tree->isUsed)
            return NULL;
        return tree;
    } else
        for (int i = 0; i < tree->childCount; ++i) {
            temp = find(tree->child[i], value);
            if (temp != NULL && temp->isUsed != true)
                return temp;
        }

    if (tree->childCount == 0)
        return NULL;

    return NULL;
}

/*
 * Проверяет, правда ли это рекурсия. Поднимается по дереву к корню, пока не встретит вершину, входящую в рекурсивную
 * цепь. Если такая вершина не была найдена, то возвращает false
 */
bool processRecursion(struct TreeNode *tree, char *value, char *parent) {
    if (tree == NULL)
        return NULL;
    if (!strcmp(tree->value, value)) {
        // Это рекурсия
        return true;
    } else if (tree->parent == NULL) {
        // Это не рекурсия
        return false;
    } else {
        // Идём в родителя
        return processRecursion(tree->parent, value, parent);
    }
}

/*
 * Очищает дерево от меток isUsed. Применяется после полной проверки на рекурсию из текущей вершины
 */
void clearTree(struct TreeNode *tree) {
    if (tree == NULL)
        return;
    tree->isUsed = false;
    for (int i = 0; i < tree->childCount; ++i) {
        clearTree(tree->child[i]);
    }
}

/*
 * Помечает дерево в лесу, как обработанное. Обработанные деревья не обрабатываются повторно в checkRecursion.
 * Используется в качестве оптимизации ненужной рекурсии, а так же предотвращает построение одинаковых деревьев
 */
void markTree(Forest *forest, const struct TreeNode *temp) {
    struct StackTreeNode *tempTree = forest->trees;
    while (tempTree != NULL) {
        if (!strcmp(tempTree->tree->value, temp->value))
            tempTree->tree->isNeeded = true;
        tempTree = tempTree->next;
    }
}

/*
 * Инициализирует новую вершину дерева и её начальные значения
 */
struct TreeNode *getNode(struct TreeNode *curNode, struct TreeNode *nodeParent, const char *value) {
    curNode = (struct TreeNode *) malloc(sizeof(struct TreeNode));
    curNode->parent = nodeParent;
    curNode->childCount = 0;
    curNode->isRecursive = false;
    curNode->isUsed = false;
    strcpy(curNode->value, value);
    if (nodeParent != NULL)
        nodeParent->child[nodeParent->childCount++] = curNode;
    return curNode;
}

/*
 * Добавляет вершину в дерево. Проверяет на наличие рекурсии
 */
struct TreeNode *
addNode(struct TreeNode *tree, struct TreeNode *curNode, struct TreeNode *nodeParent, char *value, char *parent,
        Forest *forest) {
    if (curNode == NULL) {
        curNode = getNode(curNode, nodeParent, value);
        return curNode;
    } else if (!strcmp(value, curNode->value)) {
        // Это рекурсия?
        if (processRecursion(find(tree, parent), value, parent)) {
            // Да, это рекурсия
            clearTree(tree);
            struct TreeNode *temp;

            // Пока в дереве есть вершины, которые состоят в рекурсивной цепи, проверяем их на рекурсию
            while ((temp = find(tree, parent)) != NULL)
                if (processRecursion(temp, value, parent)) {
                    // Пока не дойдём до конечной вершины в рекурсивной цепочки, будем подниматься по дереву
                    while (strcmp(temp->value, value) != 0) {
                        strcpy(tempChain.value[tempChain.size++], temp->value);

                        // Пометив дерево, как использованное в лесу. В дальнейшем мы не будем его достраивать
                        markTree(forest, temp);

                        temp->isUsed = true;
                        temp->isRecursive = true;

                        // Идём в родителя вершины (возвращаемся к истокам)
                        temp = temp->parent;
                    }
                    strcpy(tempChain.value[tempChain.size++], temp->value);

                    // Проверка, была ли эта цепочка в массиве. Если нет - добавляем
                    fillChainsArray();

                    temp->isUsed = true;
                    temp->isRecursive = true;
                } else {
                    temp->isUsed = true;
                }

            return NULL;
        }
    } else if (!strcmp(parent, curNode->value)) {
        // Мы нашли вершину, к которой нужно присобачить дитя
        // Проверим, нет ли уже этой функции в дереве
        for (int i = 0; i < curNode->childCount; ++i)
            if (!strcmp(value, curNode->child[i]->value))
                return NULL;

        // Добавляем вершину
        curNode->child[curNode->childCount] = NULL;
        addNode(tree, curNode->child[curNode->childCount], curNode, value, parent, forest);
    } else
        for (int i = 0; i < curNode->childCount; ++i)
            if (addNode(tree, curNode->child[i], curNode, value, parent, forest) == NULL)
                return NULL;

    return curNode;
}

/*
 * Строит лес. Добавляет в лес деревья с корнем, являющимся функцией, которая инициализируется в коде
 */
void
getInit(Forest *forest, char *input, int inputSize, stateTypes *now, int nowSize) {
    char word[WORDS] = {0};
    int wordSize = 0;

    for (int i = 0; i < inputSize; ++i) {
        if (isalnum(input[i]) || input[i] == '_') {
            word[wordSize++] = input[i];
        } else {
            skipComments(input, inputSize, &i, NULL);
            for (int j = 0; j < nowSize && strlen(word) != 0; ++j) {
                if (!strcmp(now[j].stateName, word) && now[j].value == INIT) {
                    // Мы прочитали тип данных: void, int ...
                    universalSkip(input, &i, inputSize, NULL);
                    clearWord(word, &wordSize);

                    // Теперь прочитаем следующее слово, считая, что дальше пойдёт имя переменной / функции
                    readWord(input, word, &wordSize, &i);

                    // Пустое слово - не инициализация вовсе
                    if (strlen(word) == 0)
                        break;

                    // Прочитали слово. Далее, смотрим, встретили ли мы '('. Если да, то это функция, добавляем её в лес
                    universalSkip(input, &i, inputSize, NULL);
                    if (input[i] == '(') {
                        struct TreeNode *tree = NULL;
                        tree = addNode(NULL, tree, NULL, word, NULL, forest);
                        pushTree(&forest, tree);
                    }
                }
            }
            clearWord(word, &wordSize);
        }
    }
}

/*
 * Добавляет в деревья леса детей первого уровня. Ищет использование функций в инициализации функций
 */
void
getUsed(Forest *forest, char *input, int inputSize, stateTypes *now, int nowSize) {
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

            // Если bracketSequence == 0, сбрасываем currentFoo
            if (bracketSequence == 0) {
                int tempSize = (int) strlen(currentFoo);
                clearWord(currentFoo, &tempSize);
            }

            skipComments(input, inputSize, &i, NULL);
            for (int j = 0; j < nowSize && strlen(word) != 0; ++j) {
                if (!strcmp(now[j].stateName, word) && now[j].value == INIT) {
                    // Мы прочитали тип данных: void, int ...
                    universalSkip(input, &i, inputSize, NULL);
                    clearWord(word, &wordSize);

                    // Теперь прочитаем следующее слово, считая, что дальше пойдёт имя переменной / функции
                    readWord(input, word, &wordSize, &i);

                    // Пустое слово - не инициализация вовсе
                    if (strlen(word) == 0)
                        break;

                    // Прочитали слово. Далее, смотрим, встретили ли мы '('. Если да, то это функция, мы в ней
                    universalSkip(input, &i, inputSize, NULL);
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
            struct StackTreeNode *foundWordTree = forest->trees;
            bool added = false;
            for (int j = 0; j < forest->size && strlen(word) != 0; ++j) {
                if (!strcmp(word, foundWordTree->tree->value)) {
                    // Да, это вызов функции
                    // Давайте найдём дерево функции, в которой мы сейчас находимся
                    struct StackTreeNode *currentFooTree = forest->trees;
                    for (int k = 0; k < forest->size; ++k) {
                        if (!strcmp(currentFoo, currentFooTree->tree->value)) {
                            // Добавляем в найденное дерево дитя
                            addNode(currentFooTree->tree, currentFooTree->tree, NULL, foundWordTree->tree->value,
                                    currentFooTree->tree->value, forest);
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

/*
 * Достраивает дерево на основе построенного леса
 */
void buildTree(Forest *forest, struct TreeNode *curTree, struct TreeNode *child) {
    // Достроим дерево, за исключением деревьев, уже учавствующих в рекурсии
    struct StackTreeNode *tempTree = forest->trees;
    while (tempTree != NULL) {
        // Мы нашли эту функцию (дитя) в списке
        if (!strcmp(tempTree->tree->value, child->value)) {
            // Рекурсивная функция уже - не берём
            if (tempTree->tree->isRecursive)
                break;

            // Переберём детей найденного дерева
            for (int i = 0; i < tempTree->tree->childCount; ++i) {
                clearTree(curTree);
                if (addNode(curTree, curTree, NULL, tempTree->tree->child[i]->value, child->value, forest) != NULL)
                    buildTree(forest, curTree, tempTree->tree->child[i]);
            }
        }
        tempTree = tempTree->next;
    }
}

/*
 * Проверка на рекурсию
 */
void checkRecursion(char *input, int inputSize, stateTypes *now, int nowSize) {
    // Лес. Корни каждого дерева - функция, которая инициализировалась пользователем
    Forest *forest = (Forest *) malloc(sizeof(Forest));
    forest->size = 0;
    forest->trees = NULL;

    getInit(forest, input, inputSize, now, nowSize);
    getUsed(forest, input, inputSize, now, nowSize);

    struct StackTreeNode *tempTree = forest->trees;
    while (tempTree != NULL) {
        // Переберём все вершины из второго уровня дерева и достроим их, если они уже не использовались
        if (!tempTree->tree->isNeeded)
            for (int i = 0; i < tempTree->tree->childCount; ++i) {
                clearTree(tempTree->tree);
                buildTree(forest, tempTree->tree, tempTree->tree->child[i]);
            }
        tempTree = tempTree->next;
    }

    // Вывод
    printChains();
}
