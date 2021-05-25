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
void insertElement(Map *m, char *key, int lineNumber, bool isFoo, char *file) {
    unsigned int id = hash(key);

    if (m[id].empty) {
        m[id].empty = 0;
        m[id].line = lineNumber;
        strcpy(m[id].key, key);
        strcpy(m[id].fileName, file);
        return;
    }

    if (isFoo)
        return;

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
void printFooMap(Map *m, char *fileName) {
    printf("\nUnused functions:\n");
    sortMap(m);
    for (int id = 0; id < MAP_SIZE; ++id)
        if (!m[id].empty && !strcmp(m[id].fileName, fileName))
            printf("Line %d: function '%s' is never used\n",  m[id].line, m[id].key);
}

/*
 * Выводим мап переменных
 */
void printVarMap(Map *m, char *fileName) {
    printf("\nUnused variables:\n");
    sortMap(m);
    for (int id = 0; id < MAP_SIZE; ++id)
        if (!m[id].empty && !strcmp(m[id].fileName, fileName))
            printf("Line %d: variable '%s' is never used\n", m[id].line, m[id].key);
}


void pushTree(Forest **forest, struct TreeNode *tree) {
    struct StackTreeNode *temp = (struct StackTreeNode *) malloc(sizeof(struct StackTreeNode));
    temp->tree = tree;
    temp->next = (*forest)->trees;
    (*forest)->trees = temp;
    (*forest)->size++;
}