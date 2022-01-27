# AVL Tree c implementation

_This is still a bit of a WIP, see TODO below_

A generic implementation of an [avl tree](https://en.wikipedia.org/wiki/AVL_tree) written in `c`.
The code is probably the best way to understand how to use it starting in `.avltest.c`.

## avlTreeType - define generic methods
```c
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
```

The `keycmp` method needs to return less than `0` for a node to be placed to
the left. Greater than 0 to be placed to the right and `0` is an exact match.
This is a function that needs to be supplied by you.

## Supported methods & types
See `avlTreeType` for the vtable used to make generic function pointers

Create a new tree specifying the type
```c
avlTree *avlTreeNew(avlTreeType *type);
```

Insert a key value into the tree returns `AVL_OK` on success
and `AVL_ERR` on failure
```c
int avlTreeInsert(avlTree *tree, void *key, void *value);
```

Remove and free a node from the tree.
```c
void avlTreeDelete(avlTree *tree, void *key);
```

Find a node in the tree ir `NULL`
```c
avlNode *avlTreeSearch(avlTree *tree, void *key);
```

Find a node and return the value from the node or `NULL`
```c
void *avlTreeGetValue(avlTree *tree, void *key);
```

Iterate over the tree
```c
void avlTreeForEach(avlTree *tree, void (*callback)(avlNode *node));
```

Best effort at printing the tree calling `avl->type-printNode` on each node
```c
void avlTreePrint(avlTree *tree);
```

## Compiling tests

Run `make` and then run `./tests.out`

## TODO:
- `avlTreeRelease` method
- `avlTreeRemove` method that returns a node to be freed by the caller
- `avlTreeForEach` make the callback method to return an int to break the loop
- more comprehensive test suite
- Better print function
