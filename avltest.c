#include <string.h>
#include <stdio.h>

#include "avl.h"

int strKeycmp(void *k1, void *k2) {
    return strcmp((const char *)k1, (const char *)k2);
}

void strPrintNode(avlNode *node) {
    if (node)
        printf("%s => %s", (char *)node->key, (char *)node->value);
}

avlTreeType string_table = {
    .keycmp = strKeycmp,
    .freeKey = NULL,
    .freeValue = NULL,
    .printNode = strPrintNode,
};

int main(void) {
    avlTree *tree;
    tree = avlTreeNew(&string_table);
    avlTreeInsert(tree, "hello", "world");
    avlTreeInsert(tree, "hey", "there");
    avlTreeInsert(tree, "whats", "up");
    avlTreeInsert(tree, "how're", "you?");
    printf("inserted\n");

    avlTreePrint(tree);
    avlTreeDelete(tree, "whats");
    printf("remove\n");
    avlTreePrint(tree);

    char *value = avlTreeGetValue(tree, "hey8asd8as8d");
    if (value)
        printf("%s\n", value);
}
