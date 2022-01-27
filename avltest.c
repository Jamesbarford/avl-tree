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
    tree = avlNew(&string_table);
    avlInsert(tree, "hello", "world");
    avlInsert(tree, "hey", "there");
    avlInsert(tree, "whats", "up");
    avlInsert(tree, "how're", "you?");
    printf("inserted\n");

    avlPrint(tree);
    printf("size => %u\n", tree->size);
    printf("remove\n");
    avlDelete(tree, "whats");
    avlPrint(tree);

    avlRelease(tree);
}
