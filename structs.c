#include "structs.h"

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


/*
 * Функция хеширования
 */
unsigned int hash(char key[KEY_SIZE]) {
    unsigned long long result = 0;
    unsigned long long multiplier = 1;
    for (int i = 0; i < strlen(key); ++i) {
        result = (result + (unsigned long long) key[i] * multiplier) % MAP_SIZE;
        multiplier *= HASH_MULTIPLIER;
    }
    return result;
}

/*
 * Инициализация мапа
 */
void initElements(Map *m) {
    for (int i = 0; i < MAP_SIZE; ++i) {
        m[i].key[0] = 0;
        m[i].empty = true;
    }
}

/*
 * Вносит элемент в мап
 */
void insertElement(Map *m, char *key, int lineNumber) {
    unsigned int id = hash(key);

    if (m[id].empty) {
        m[id].empty = 0;
        strcpy(m[id].key, key);
        m[id].line = lineNumber;
        return;
    }

    unsigned int start = id;
    id = (id + 1) % MAP_SIZE;
    while (id != start)
        if (m[id].empty) {
            m[id].empty = 0;
            strcpy(m[id].key, key);
            m[id].line = lineNumber;
            return;
        } else
            id = (id + 1) % MAP_SIZE;

    printf("Critical error: unable to insertElement() - possible overflow");
}

/*
 * Проверяет, есть ли данный элемент в мапе, а если есть, то устанавливает empty
 */

// TODO!!! изменить поиск элемента. Пример в input'e
// upd. сделано вроде?
void checkElement(Map *m, char *key) {
    unsigned int id = hash(key);

    unsigned int tempStart = id;
    while (!m[id + 1].empty && !strcmp(m[id + 1].key, key) && (id + 1) != tempStart) {
        id = (id + 1) % MAP_SIZE;
    }

    if (!m[id].empty && !strcmp(m[id].key, key)) {
        m[id].empty = true;
        return;
    }

    unsigned int start = id;
    id = (id + 1) % MAP_SIZE;
    while (id != start)
        if (!m[id].empty && !strcmp(m[id].key, key))
            m[id].empty = true;
        else
            id = (id + 1) % MAP_SIZE;
}

/*
 * Сортировка мапа вставками
 */
void sortMap(Map *m) {
    for (int i = 1; i < MAP_SIZE; ++i) {
        int j = i - 1;
        while (j >= 0 && (m[j].line > m[j + 1].line || m[j].empty)) {
            Map temp = m[j];
            m[j] = m[j + 1];
            m[j + 1] = temp;
            --j;
        }
    }
}

/*
 * Выводим мап функций
 */
void printFooMap(Map *m) {
    sortMap(m);
    for (int id = 0; id < MAP_SIZE; ++id)
        if (!m[id].empty)
            printf("Line %d: function '%s' is never used\n", m[id].line, m[id].key);
}

/*
 * Выводим мап переменных
 */
void printVarMap(Map *m) {
    sortMap(m);
    for (int id = 0; id < MAP_SIZE; ++id)
        if (!m[id].empty)
            printf("Line %d: variable '%s' is never used\n", m[id].line, m[id].key);
}


void pushTree(Forest **forest, struct TreeNode *tree) {
    /*if ((*forest)->trees == NULL)
        (*forest)->trees->next = NULL;*/
    struct stackTreeNode *temp = (struct stackTreeNode *) malloc(sizeof(struct stackTreeNode));
    temp->tree = tree;
    temp->next = (*forest)->trees;
    (*forest)->trees = temp;
    (*forest)->size++;
}




struct TreeNode *find(struct TreeNode *tree, char *value) {
    struct TreeNode *temp;
    if (!strcmp(tree->value, value)) {
        if (tree->isUsed)
            return NULL;
        return tree;
    } else {
        for (int i = 0; i < tree->childCount; ++i) {
            temp = find(tree->child[i], value);
            if (temp != NULL && temp->isUsed != true) {
                //temp->isUsed = true;
                return temp;
            }
        }
    }

    if (tree->childCount == 0)
        return NULL;

    return NULL;
}

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
        processRecursion(tree->parent, value, parent);
    }
}

void clearTree(struct TreeNode *tree) {
    if (tree == NULL)
        return;
    tree->isUsed = false;
    for (int i = 0; i < tree->childCount; ++i) {
        clearTree(tree->child[i]);
    }
}

struct TreeNode *
addNode(struct TreeNode *tree, struct TreeNode *curNode, struct TreeNode *nodeParent, char *value, char *parent,
        Forest *forest) {
    if (curNode == NULL) {
        curNode = (struct TreeNode *) malloc(sizeof(struct TreeNode));
        curNode->parent = nodeParent;
        curNode->childCount = 0;
        curNode->isRecursive = false;
        curNode->isUsed = false;
        strcpy(curNode->value, value);
        if (nodeParent != NULL)
            nodeParent->child[nodeParent->childCount++] = curNode;
        return curNode;
    } else if (!strcmp(value, curNode->value)) {
        // Это рекурсия?
        if (processRecursion(find(tree, parent), value, parent)) {
            //find(tree, parent)->isRecursive = true;
            clearTree(tree);
            struct TreeNode *temp;

            while ((temp = find(tree, parent)) != NULL) {
                if (processRecursion(temp, value, parent)) {
                    //printf("Recursion chain: ");
                    while (strcmp(temp->value, value) != 0) {
                        //printf("%s - ", temp->value);
                        //
                        strcpy(tempChain.value[tempChain.size++], temp->value);

                        // Пометив вершину, как рекурсивную в лесу
                        struct stackTreeNode *tempTree = forest->trees;
                        while (tempTree != NULL) {
                            if (!strcmp(tempTree->tree->value, temp->value))
                                tempTree->tree->isNeeded = true;
                            tempTree = tempTree->next;
                        }

                        temp->isUsed = true;
                        temp->isRecursive = true;
                        temp = temp->parent;
                    }
                    strcpy(tempChain.value[tempChain.size++], temp->value);

                    // Проверка
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

                    //printf("%s\n", temp->value);
                    temp->isUsed = true;
                    temp->isRecursive = true;
                } else {
                    temp->isUsed = true;
                }
            }

            return NULL;
        }
    } else if (!strcmp(parent, curNode->value)) {
        // Мы нашли вершину, к которой нужно присобачить дитя
        // Проверим, нет ли уже этой функции в дереве
        for (int i = 0; i < curNode->childCount; ++i)
            if (!strcmp(value, curNode->child[i]->value))
                return NULL;

        curNode->child[curNode->childCount] = NULL;
        addNode(tree, curNode->child[curNode->childCount], curNode, value, parent, forest);
    } else {
        for (int i = 0; i < curNode->childCount; ++i) {
            if(addNode(tree, curNode->child[i], curNode, value, parent, forest) == NULL)
                return NULL;
        }
    }

    return curNode;
}