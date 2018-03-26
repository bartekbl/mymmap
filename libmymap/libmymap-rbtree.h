#ifndef LIBMYMMAPRBTREE_H
#define LIBMYMMAPRBTREE_H

enum RBTreeError
{
    RBTREE_NO_ERROR                   =  0,
    RBTREE_INSERT_ERROR_CANT_ALLOCATE = -1,
    RBTREE_REMOVE_ERROR_NOT_FOUND     = -2,
    RBTREE_INSERT_ERROR_DUPLICATE     = -3,
};

enum RBTreeColor
{
    RBTREE_COLOR_RED,
    RBTREE_COLOR_BLACK
};

struct RBTree
{
    struct RBTree*   left;
    struct RBTree*   right;
    struct RBTree*   parent;
    enum RBTreeColor color;
//    int              value;
};

enum RBTreeError rbtree_init(struct RBTree** tree);

// insert is impossible here because we don't know what value is stored, its size or how it is compared
// provide your own insert with binary-search-tree insert and use rbtree_corrext
//enum RBTreeError rbtree_insert(struct RBTree** tree,
//                               int             value,
//                               int             debug_info);

void rbtree_correct(struct RBTree** tree,
                    struct RBTree* new_node);

// in order to remove the node you must first find it yourself
// than rbtree_remove will take care of balancing the tree
// last but not least you must free it yourself
enum RBTreeError rbtree_remove(struct RBTree** tree,
                               struct RBTree*  node);

//void rbtree_destroy(struct RBTree** tree);

#endif // LIBMYMMAPRBTREE_H
