#ifndef __AVL_H__
#define __AVL_H__

#define AVL_ERR 0
#define AVL_OK 1

struct avlNode;

typedef struct avlNode {
    void *key;
    void *value;
    int height;
    struct avlNode *left;
    struct avlNode *right;
} avlNode;

typedef struct avlTreeType {
    /* Free the key of the avl node */
    void (*freeKey)(void *);
    /* Free the value of the avl node */
    void (*freeValue)(void *);
    /* Print the node */
    void (*printNode)(avlNode *);
    /**/
    int (*keycmp)(void *, void *);
} avlTreeType;

typedef struct avlTree {
    avlNode *root;
    unsigned int size;
    avlTreeType *type;
} avlTree;

avlTree *avlTreeNew(avlTreeType *type);
int avlTreeInsert(avlTree *tree, void *key, void *value);
void avlTreeDelete(avlTree *tree, void *key);
avlNode *avlTreeSearch(avlTree *tree, void *key);
void *avlTreeGetValue(avlTree *tree, void *key);
void avlTreeForEach(avlTree *tree, void (*callback)(avlNode *node));
void avlTreePrint(avlTree *tree);
void avlTreeRelease(avlTree *tree);

#endif
