#include "libmymap-rbtree.h"
#include "libmymap-rbtree-dump.h"
#include "libmymap-rbtree-internal.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

enum RBTreeError rbtree_init(struct RBTree** tree)
{
    *tree = NULL;
    return 0;
}

struct RBTree** rbtree_get_pointer_to_parent_pointer_to_node(struct RBTree** tree, struct RBTree* node)
{
    struct RBTree* parent = node->parent;
    if (parent == NULL)
    {
        assert (node == *tree);
        return tree;
    }
    if (parent->right == node) return &parent->right;
    else
    {
        assert (parent->left == node);
        return &parent->left;

    }
}

// it takes pointer to a parent of rotated pair and returns new parent
struct RBTree* rbtree_rotate_left(struct RBTree** tree, struct RBTree* parent)
{
    // parent      - parent node being rotated
    // child       - child node being rotated
    // grandparent - parent of a parent that stays stationary but has its child exchanged
    // grandchild  - subtree that will have its parent exchanged
    assert(parent        != NULL);
    struct RBTree* grandparent = parent->parent;
    struct RBTree* child = parent->right; // it must not be NULL for rotation to work
    assert(child != NULL);
    struct RBTree* grandchild = child->left;

    struct RBTree** parent_pointer = rbtree_get_pointer_to_parent_pointer_to_node(tree, parent);
    *parent_pointer = child;

    child->parent = grandparent;

    parent->parent = child;
    child->left = parent;

    if (grandchild != NULL) grandchild->parent = parent;
    parent->right = grandchild;

    return child;
}

// it takes pointer to a parent of rotated pair and returns new parent
struct RBTree* rbtree_rotate_right(struct RBTree** tree, struct RBTree* parent)
{
    // parent      - parent node being rotated
    // child       - child node being rotated
    // grandparent - parent of a parent that stays stationary but has its child exchanged
    // grandchild  - subtree that will have its parent exchanged
    assert(parent        != NULL);
    struct RBTree* grandparent = parent->parent;
    struct RBTree* child = parent->left; // it must not be NULL for rotation to work
    assert(child != NULL);
    struct RBTree* grandchild = child->right;

    struct RBTree** parent_pointer = rbtree_get_pointer_to_parent_pointer_to_node(tree, parent);
    *parent_pointer = child;

    child->parent = grandparent;

    parent->parent = child;
    child->right = parent;

    if (grandchild != NULL) grandchild->parent = parent;
    parent->left = grandchild;

    return child;
}

//void rbtree_destroy(struct RBTree** tree)
//{
//    struct RBTree* cursor = *tree;
//    while (*tree != NULL)
//    {
//        if      (cursor->left  != NULL) cursor = cursor->left;
//        else if (cursor->right != NULL) cursor = cursor->right;
//        else
//        {
//            struct RBTree* parent = cursor->parent;
//            // Here is dengerous part. Cursor is already freed but either parent->left or parent->right is still pointing to it.
//            if (parent == NULL) assert (cursor == *tree);
//            struct RBTree** parent_pointer = rbtree_get_pointer_to_parent_pointer_to_node(tree, cursor);
//            *parent_pointer = NULL;
//            free(cursor);
//            cursor = parent;
//        }
//    }
//}
