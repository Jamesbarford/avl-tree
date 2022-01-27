#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "avl.h"

#ifdef AVL_DEBUG
#define __AVL_DEBUG 1
#else
#define __AVL_DEBUG 0
#endif

#define avlDebug(...)                                                          \
    do {                                                                       \
        if (__AVL_DEBUG) fprintf(stderr, __VA_ARGS__);                         \
    } while (0)

#define _avlMax(a, b) ((a) > (b) ? (a) : (b))
#define _avlNodeHeight(n) ((n) != NULL ? (n)->height : 0)
#define _avlBalance(n)                                                         \
    ((n) != NULL ? _avlNodeHeight((n)->left) - _avlNodeHeight((n)->right) : 0)

#define _avlPrintNode(t, n)                                                    \
    ((t)->type != NULL && (t)->type->printNode ? (t)->type->printNode(n)       \
                                               : (void)n)
#define _avlFreeKey(tree, key)                                                 \
    ((tree)->type != NULL && (tree)->type->freeKey != NULL                     \
                    ? (tree)->type->freeKey((key))                             \
                    : (void)key)
#define _avlFreeValue(tree, key)                                               \
    ((tree)->type != NULL && (tree)->type->freeKey != NULL                     \
                    ? (tree)->type->freeKey((key))                             \
                    : (void)key)
#define _avlKeyCmp(tree, k1, k2)                                               \
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

static avlNode *_rightRotate(avlNode *node) {
    avlNode *n2 = node->left;
    avlNode *next_node = n2->right;
    n2->right = node;
    node->left = next_node;
    node->height = _avlMax(_avlNodeHeight(node->left), _avlNodeHeight(node->right)) + 1;
    n2->height = _avlMax(_avlNodeHeight(n2->left), _avlNodeHeight(n2->right)) + 1;
    return n2;
}

static avlNode *_leftRotate(avlNode *node) {
    avlNode *n2 = node->right;
    avlNode *next_node = n2->left;
    n2->left = node;
    node->right = next_node;
    node->height = _avlMax(_avlNodeHeight(node->left), _avlNodeHeight(node->right)) + 1;
    n2->height = _avlMax(_avlNodeHeight(n2->left), _avlNodeHeight(n2->right)) + 1;
    return n2;
}

/**
 * Insert a key value into the tree, exists will be set to 0 if the key
 * already exists within the tree.
 */
static avlNode *_avlNodeInsert(avlTree *tree, avlNode *node, void *key,
        void *value, int *exists)
{
    if (node == NULL) {
        return _avlNodeNew(key, value);
    }

    int cmpval, balance, right_cmp, left_cmp;

    cmpval = _avlKeyCmp(tree, key, node->key);

    if (cmpval < 0) {
        node->left = _avlNodeInsert(tree, node->left, key, value, exists);
    } else if (cmpval > 0) {
        node->right = _avlNodeInsert(tree, node->right, key, value, exists);
    } else {
        /**
         * TODO: decide on whether to free(old_value) and set to the new
         * value being passed in.
         *
         * node exists, so nothing new was created.
         */
        exists = 0;
        return node;
    }

    node->height = 1 + _avlMax(_avlNodeHeight(node->left),
                               _avlNodeHeight(node->right));
    balance = _avlBalance(node);

    if (balance > 1) {
        left_cmp = _avlKeyCmp(tree, key, node->left->key);
        if (left_cmp < 0) {
            return _rightRotate(node);
        } else if (left_cmp > 0) {
            node->left = _leftRotate(node->left);
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

/**
 * Remove and free a node in the tree. The key and value get assigned to 
 * the stack allocated node to release. Due to the rotations it is a bit
 * tricky trying to keep track of it.
 *
 * We determine if we need to assing to stack node predicated on the 
 * key pointer being NULL.
 */
static avlNode *_avlNodeDelete(avlTree *tree, avlNode *node, void *key,
        avlNode *stack_node, int *has_freed)
{
    if (tree->size == 1) {
        *stack_node = *tree->root;
        free(tree->root);
        *has_freed = 1;
        /* we'll assign this to the root in the public method*/
        return NULL;
    }

    int cmpval, balance;
    avlNode *tmp;

    cmpval = _avlKeyCmp(tree, key, node->key);

    if (cmpval < 0) {
        node->left = _avlNodeDelete(tree, node->left, key, stack_node, has_freed);
    } else if (cmpval > 0) {
        node->right = _avlNodeDelete(tree, node->right, key, stack_node, has_freed);
        /* exact match */
    } else if (cmpval == 0) {
        if ((node->left == NULL) || (node->right == NULL)) {
            tmp = node->left != NULL ? node->left : node->right;
            if (tmp == NULL) {
                tmp = node;
                node = NULL;
            } else {
                /* Before we lose track of what we are freeing assign to stack_node */
                if (stack_node->key == NULL)
                    *stack_node = *node;
                *node = *tmp;
            }

            /**
             * TODO:
             * Assign the node here for the caller to handle... 
             * I cannot get this node out of this mess.
             *
             * The best I seem to be able to do is keep track of the actual
             * values we are trying to free via stack_node, is it even possible
             * to return the node?
             */
            *has_freed = 1;
            free(tmp);
        } else {
            /* Before we lose track of what we are freeing assign to stack_node */
            if (stack_node->key == NULL)
                *stack_node = *node;
            tmp = _avlNodeWithMinimumValue(node->right);
            node->key = tmp->key;
            node->value = tmp->value;
            node->right = _avlNodeDelete(tree, node->right, tmp->key,
                    stack_node, has_freed);
        }
    } else {
        /* No match, can we even get here? */
        return NULL;
    }

    if (node == NULL)
        return NULL;

    node->height = 1 + _avlMax(_avlNodeHeight(node->left),
                               _avlNodeHeight(node->right));
    balance = _avlBalance(node);

    if (balance > 1) {
        if (_avlBalance(node->left) >= 0) {
            return _rightRotate(node);
        } else {
            node->left = _leftRotate(node->left);
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
    int exists;

    /* We assign this to 0 if a node already exists with this key */
    exists = 1;

    if ((new_root = _avlNodeInsert(tree, tree->root, key, value, &exists)) == NULL)
        return AVL_ERR;

    /* if the key has been newly inserted increment the size */
    if (exists == 1)
        tree->size++;

    tree->root = new_root;

    return AVL_OK;
}

/**
 * Remove the node from the tree and free the key, value and node.
 */
void avlDelete(avlTree *tree, void *key) {
    avlNode *new_root, stack_node;
    int has_freed;

    has_freed = 0;

    stack_node.key = NULL;
    stack_node.value = NULL;
    new_root = _avlNodeDelete(tree, tree->root, key, &stack_node, &has_freed);

    /**
     * If we were able to free the node we can now free the key and value
     */
    if (has_freed == 1) {
        tree->size--;
        /* Upto caller to decide if the value is NULL or not */
        _avlFreeKey(tree, stack_node.key);
        _avlFreeValue(tree, stack_node.value);
    }

    /* There was no match */
    if (new_root == NULL) {
        tree->root = new_root;
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
    avlDebug("size => %u\n", tree->size);
    while (tree->root != NULL) {
        avlDebug("free(%s => %s)\tsize => %u\n",
                (char *)tree->root->key, (char *)tree->root->value,
                tree->size);
        avlDelete(tree, tree->root->key);
    }
    free(tree);
}
