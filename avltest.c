#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "avl.h"

int strKeycmp(void *k1, void *k2) {
    return strcmp((const char *)k1, (const char *)k2);
}

void strPrintNode(avlNode *node) {
    if (node)
        printf("%s => %s", (char *)node->key, (char *)node->value);
}

void tryFree(void *__v) {
    if (__v)
        free(__v);
}

avlTreeType string_table = {
        .keycmp = strKeycmp,
        .freeKey = NULL,
        .freeValue = NULL,
        .printNode = strPrintNode,
};

avlTreeType malloced_string_table = {
        .keycmp = strKeycmp,
        .freeKey = tryFree,
        .freeValue = tryFree,
        .printNode = strPrintNode,
};

struct K_V {
    char *key;
    char *value;
} test_table[] = {
        {"foo", "bar"},
        {"key", "value"},
        {"next", "item"},
        {"yeeet", "gta"},
        {"revolut", "bank"},
        {"debit", "card"},
        {"empty", "bowling"},
        {NULL, NULL},
};

/**
 * Very simple test, ensure every value gets set and nothing seg faults
 * and then destroy every value
 */
int testFreeingKeysAndValues(void) {
    avlTree *tree = avlNew(&malloced_string_table);
    struct K_V *ptr = test_table;

    while (ptr != NULL && ptr->key != NULL) {
        char *key = strdup(ptr->key);
        char *value = strdup(ptr->value);

        avlInsert(tree, key, value);
        ptr++;
    }

    avlPrint(tree);
    avlRelease(tree);

    return AVL_OK;
}

int main(void) {
    avlTree *tree;
    tree = avlNew(&string_table);
    testFreeingKeysAndValues();
    avlInsert(tree, "hello", "world");
    avlInsert(tree, "hey", "there");
    avlInsert(tree, "whats", "up");
    avlInsert(tree, "how're", "you?");
    printf("inserted\n");

    avlPrint(tree);
    avlDelete(tree, "whats");
    avlPrint(tree);
    avlRelease(tree);
}
