#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "avl.h"

#define _avlMax(a, b) ((a) > (b) ? (a) : (b))
#define _avlNodeHeight(n) ((n) != NULL ? (n)->height : 0)
#define _avlBalance(n)                                                         \
    ((n) != NULL ? _avlNodeHeight((n)->left) - _avlNodeHeight((n)->right) : 0)

#define _avlPrintNode(t, n)                                                    \
    ((t)->type != NULL && (t)->type->printNode ? (t)->type->printNode(n)       \
                                               : (void)n)
#define _avlFreeKey(tree, key)                                             \
    ((tree)->type != NULL && (tree)->type->freeKey != NULL                     \
                    ? (tree)->type->freeKey((key))                             \
                    : (void)key)
#define _avlFreeValue(tree, key)                                           \
    ((tree)->type != NULL && (tree)->type->freeKey != NULL                     \
                    ? (tree)->type->freeKey((key))                             \
                    : (void)key)
#define _avlKeyCmp(tree, k1, k2)                                           \
    ((tree)->type != NULL && (tree)->type->keycmp != NULL                      \
                    ? (tree)->type->keycmp((k1), (k2))                         \
                    : -1)

static avlNode *_avlNodeNew(void *key, void *value) {
    avlNode *node;

    if ((node = malloc(sizeof(avlNode))) == NULL)
        return NULL;

    node->height = 1;
    node->key = key;
    node->value = value;
    node->left = NULL;
    node->right = NULL;

    return node;
}

static void _avlNodeRelease(avlTree *tree, avlNode *node) {
    if (node) {
        _avlFreeKey(tree, node->key);
        _avlFreeValue(tree, node->key);
        free(node);
    }
}

static avlNode *_rightRotate(avlNode *y) {
    avlNode *l = y->left;
    avlNode *r = l->right;
    l->right = y;
    y->left = r;
    y->height = _avlMax(_avlNodeHeight(y->left), _avlNodeHeight(y->right)) + 1;
    l->height = _avlMax(_avlNodeHeight(l->left), _avlNodeHeight(l->right)) + 1;
    return l;
}

static avlNode *_leftRotate(avlNode *x) {
    avlNode *r = x->right;
    avlNode *l = r->left;
    r->left = x;
    x->right = l;
    x->height = _avlMax(_avlNodeHeight(x->left), _avlNodeHeight(x->right)) + 1;
    r->height = _avlMax(_avlNodeHeight(r->left), _avlNodeHeight(r->right)) + 1;
    return r;
}

static avlNode *_avlNodeInsert(avlTree *tree, avlNode *node, void *key,
        void *value)
{
    if (node == NULL)
        return _avlNodeNew(key, value);

    int cmpval, balance, right_cmp, left_cmp;

    cmpval = _avlKeyCmp(tree, key, node->key);

    if (cmpval < 0)
        node->left = _avlNodeInsert(tree, node->left, key, value);
    else if (cmpval > 0)
        node->right = _avlNodeInsert(tree, node->right, key, value);
    else
        return node;

    node->height = 1 + _avlMax(_avlNodeHeight(node->left),
                               _avlNodeHeight(node->right));
    balance = _avlBalance(node);

    if (balance > 1) {
        left_cmp = _avlKeyCmp(tree, key, node->left->key);
        if (left_cmp < 0) {
            return _rightRotate(node);
        } else if (left_cmp > 0) {
            node->left = _rightRotate(node->left);
            return _rightRotate(node);
        }
    }

    if (balance < -1) {
        right_cmp = _avlKeyCmp(tree, key, node->right->key);
        if (right_cmp > 0) {
            return _leftRotate(node);
        } else if (right_cmp < 0) {
            node->right = _rightRotate(node->right);
            return _leftRotate(node);
        }
    }

    return node;
}

static avlNode *_avlNodeWithMinimumValue(avlNode *node) {
    avlNode *cur = node;
    while (cur->left != NULL)
        cur = cur->left;
    return cur;
}

static avlNode *_avlNodeDelete(avlTree *tree, avlNode *node, void *key) {
    if (tree->size == 1) {
        _avlNodeRelease(tree, tree->root);
        tree->size = 0;
        /* we'll assign this to the root in the public method*/
        return NULL;
    }

    int cmpval, balance;
    avlNode *tmp;

    cmpval = _avlKeyCmp(tree, key, node->key);

    if (cmpval < 0) {
        node->left = _avlNodeDelete(tree, node->left, key);
    } else if (cmpval > 0) {
        node->right = _avlNodeDelete(tree, node->right, key);
        /* exact match */
    } else if (cmpval == 0) {
        if ((node->left == NULL) || (node->right == NULL)) {
            tmp = node->left != NULL ? node->left : node->right;
            if (tmp == NULL) {
                tmp = node;
                node = NULL;
            } else {
                *node = *tmp;
            }
            tree->size--;
            _avlNodeRelease(tree, tmp);
        } else {
            tmp = _avlNodeWithMinimumValue(node->right);
            node->key = tmp->key;
            node->value = tmp->value;
            node->right = _avlNodeDelete(tree, node->right, tmp->key);
        }
    } else {
        /* No match */
        return NULL;
    }

    if (node == NULL)
        return NULL;

    tree->size++;
    node->height = 1 + _avlMax(_avlNodeHeight(node->left),
                               _avlNodeHeight(node->right));
    balance = _avlBalance(node);

    if (balance > 1) {
        if (_avlBalance(node->left) >= 0) {
            return _rightRotate(node);
        } else {
            node->left = _leftRotate(node);
            return _rightRotate(node);
        }
    }

    if (balance < -1) {
        if (_avlBalance(node->right) <= 0) {
            return _leftRotate(node);
        } else {
            node->right = _rightRotate(node->right);
            return _leftRotate(node);
        }
    }

    return node;
}

static void _avlPrint(avlTree *tree, avlNode *node, char *indent, int last) {
    if (node != NULL) {
        printf("%s", indent);

        if (last) {
            printf("R----");
            strcat(indent, "   ");
        } else {
            printf("L----");
            strcat(indent, "|  ");
        }

        _avlPrintNode(tree, node);
        printf("\n");

        _avlPrint(tree, node->left, indent, 0);
        _avlPrint(tree, node->right, indent, 1);
    }
}

static avlNode *_avlSearch(avlTree *tree, avlNode *node, void *key) {
    if (node == NULL)
        return NULL;

    int cmpval;

    cmpval = _avlKeyCmp(tree, key, node->key);

    if (cmpval == 0) {
        return node;
    } else if (cmpval < 0) {
        node = _avlSearch(tree, node->left, key);
    } else {
        node = _avlSearch(tree, node->right, key);
    }

    /* NOT REACHED */
    return node;
}

/* public methods */

int avlInsert(avlTree *tree, void *key, void *value) {
    avlNode *new_root;

    if ((new_root = _avlNodeInsert(tree, tree->root, key, value)) == NULL)
        return AVL_ERR;

    tree->root = new_root;

    return AVL_OK;
}

void avlDelete(avlTree *tree, void *key) {
    avlNode *new_root;
    new_root = _avlNodeDelete(tree, tree->root, key);

    /* There was no match */
    if (new_root == NULL && tree->size >= 1)
        return;

    /* tree is empty */
    if (tree->size == 0) {
        tree->root = NULL;
        return;
    }

    /* successful delete */
    tree->root = new_root;
}

avlNode *avlSearch(avlTree *tree, void *key) {
    avlNode *found;

    if ((found = _avlSearch(tree, tree->root, key)) == NULL)
        return NULL;

    return found;
}

void *avlGetValue(avlTree *tree, void *key) {
    avlNode *found;

    if ((found = _avlSearch(tree, tree->root, key)) == NULL)
        return NULL;

    return found->value;
}

void avlPrint(avlTree *tree) {
    char indent[BUFSIZ] = {0};
    _avlPrint(tree, tree->root, indent, 1);
}

void _avlForEach(avlNode *node, void (*callback)(avlNode *node)) {
    if (node != NULL) {
        callback(node);
        _avlForEach(node->left, callback);
        _avlForEach(node->right, callback);
    }
}

void avlForEach(avlTree *tree, void (*callback)(avlNode *node)) {
    _avlForEach(tree->root, callback);
}

avlTree *avlNew(avlTreeType *type) {
    avlTree *tree;
    if ((tree = malloc(sizeof(avlTree))) == NULL)
        return NULL;
    tree->type = type;
    tree->size = 0;
    tree->root = NULL;
    return tree;
}

/**
 * Free all the nodes, keys, values and tree. The tree cannot be used
 * after this function is used
 */
void avlRelease(avlTree *tree) {
    while (tree->root != NULL)
        avlDelete(tree, tree->root->key);
    free(tree);
}
