#include "libmymap-rbtree-dump.h"
#include "libmymap-rbtree-internal.h"
#include "libmymap-rbtree.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

static void rbtree_remove_swap_remote_left_subtree(struct RBTree** tree, struct RBTree* node, struct RBTree* descendant)
{
    assert (descendant->parent       != NULL      ); // we use "rbtree_remove_swap_remote_left_subtree function" only in one place in "remove" function so this assertion is safe
    assert (descendant->parent->left == descendant); // we use "rbtree_remove_swap_remote_left_subtree function" only in one place in "remove" function so this assertion is safe
    assert (descendant->left         == NULL      ); // we use "rbtree_remove_swap_remote_left_subtree function" only in one place in "remove" function so this assertion is safe
    assert (node->left               != NULL      ); // we use "rbtree_remove_swap_remote_left_subtree function" only in one place in "remove" function so this assertion is safe
    assert (node->right              != NULL      ); // we use "rbtree_remove_swap_remote_left_subtree function" only in one place in "remove" function so this assertion is safe

    struct RBTree* temp;

    // fix incoming parent pointers
    assert (descendant->parent->left == descendant);
    descendant->parent->left = node;
    struct RBTree** parent_pointer = rbtree_get_pointer_to_parent_pointer_to_node(tree, node);
    *parent_pointer = descendant;
    // swap outgoing parent pointers
    temp = node->parent;
    node->parent = descendant->parent;
    descendant->parent = temp;

    // fix incoming right pointers
    assert(node->right != NULL);
    node->right->parent = descendant;
    if (descendant->right != NULL) descendant->right->parent = node;
    // swap outgoing right pointers
    temp = node->right;
    node->right = descendant->right;
    descendant->right = temp;

    // fix incoming left pointers
    node->left->parent = descendant;
    // swap outgoing left pointers
    descendant->left = node->left;
    node->left = NULL;

    enum RBTreeColor tempc = node->color;
    node->color = descendant->color;
    descendant->color = tempc;
}

static void rbtree_remove_swap_adjacent_right_child(struct RBTree** tree, struct RBTree* node, struct RBTree* child)
{
    assert (child->parent == node);
    assert (node->right == child);
    assert (child->left == NULL); // we use "rbtree_remove_swap_adjacent_right_child function" only in one place in "remove" function so this assertion is safe
    assert (node->left != NULL); // we use "rbtree_remove_swap_adjacent_right_child function" only in one place in "remove" function so this assertion is safe

    // fix incoming parent pointers
    struct RBTree** parent_pointer = rbtree_get_pointer_to_parent_pointer_to_node(tree, node);
    *parent_pointer = child;
    // swap outgoing parent pointers
    child->parent = node->parent;
    node->parent = child;

    // fix incoming right pointers
    if (child->right != NULL) child->right->parent = node;
    // swap outgoing right pointers
    node->right = child->right;
    child->right = node;

    // fix incoming left pointers
    node->left->parent = child;
    // swap outgoing left pointers
    child->left = node->left;
    node->left = NULL;

    enum RBTreeColor temp = node->color;
    node->color = child->color;
    child->color = temp;
}

static void rbtree_remove_find_replacement_and_swap(struct RBTree** tree, struct RBTree* cursor)
{
    struct RBTree* replacement = cursor->right;
    if (replacement->left == NULL) // swap adjecent nodes
    {
        rbtree_remove_swap_adjacent_right_child(tree, cursor, replacement);
    }
    else // find leftmost node and swap separate nodes
    {
        while (replacement->left != NULL) replacement = replacement->left;
        rbtree_remove_swap_remote_left_subtree(tree, cursor, replacement);
    }
}

static enum RBTreeError rbtree_remove_cut_out_black_node(struct RBTree** tree, struct RBTree* cursor)
{
    assert (cursor->left == NULL || cursor->right == NULL);
    assert (cursor->left != NULL || cursor->right != NULL);

    struct RBTree* child = cursor->left;
    if (child == NULL) child = cursor->right;

    assert (child != NULL);
    assert (child->color == RBTREE_COLOR_RED);

    child->parent = cursor->parent;

    struct RBTree** parent_pointer = rbtree_get_pointer_to_parent_pointer_to_node(tree, cursor);
    *parent_pointer = child;

    child->color = RBTREE_COLOR_BLACK;
    return RBTREE_NO_ERROR;
}

static enum RBTreeError rbtree_remove_free_residual_node_and_terminate(struct RBTree** tree, struct RBTree* to_delete)
{
//    assert (to_delete->parent != NULL);
    assert (to_delete->left == NULL);
    assert (to_delete->right == NULL);

    struct RBTree** parent_pointer = rbtree_get_pointer_to_parent_pointer_to_node(tree, to_delete);
    *parent_pointer = NULL;

    return RBTREE_NO_ERROR;
}

enum RBTreeError rbtree_remove(struct RBTree** tree,
                               struct RBTree*  node)
{
    assert (tree != NULL); // we must be given valid pointer to a pointer to a tree
    assert (*tree == NULL || (*tree)->parent == NULL); // tree can be empty or we can operate on root but we can't start from the middle

    struct RBTree* cursor = node;
//    struct RBTree* cursor = *tree;

//    while (1)
//    {
//        if (cursor == NULL) return RBTREE_REMOVE_ERROR_NOT_FOUND;

//        if (value == cursor->value) break; // Found it!

//        if (value < cursor->value) cursor = cursor->left;
//        else                       cursor = cursor->right;
//    }

    if (cursor->left != NULL && cursor->right != NULL) rbtree_remove_find_replacement_and_swap(tree, cursor);

    assert (cursor->left == NULL || cursor->right == NULL);
    if (cursor->color == RBTREE_COLOR_RED)
    {
        return rbtree_remove_free_residual_node_and_terminate(tree, cursor);
    }
    if (cursor->color == RBTREE_COLOR_BLACK && (cursor->left != NULL || cursor->right != NULL))
    {
        return rbtree_remove_cut_out_black_node(tree, cursor);
    }

    assert (cursor->left == NULL);
    assert (cursor->right == NULL);
    assert (cursor->color == RBTREE_COLOR_BLACK);
    // delete lone black node
    struct RBTree* to_delete = cursor;
    if (cursor->parent == NULL) return rbtree_remove_free_residual_node_and_terminate(tree, to_delete);
    while (1)
    {
        struct RBTree* parent = cursor->parent;
        if (parent == NULL)
        {
            assert (*tree == cursor);
            assert (cursor != to_delete);
            return rbtree_remove_free_residual_node_and_terminate(tree, to_delete);
        }
        if (parent->right == cursor)
        {
            struct RBTree* sibling = parent->left;
            assert (sibling != NULL);
            if (sibling->color == RBTREE_COLOR_RED)
            {
                sibling->color = RBTREE_COLOR_BLACK;
                parent->color = RBTREE_COLOR_RED;
                rbtree_rotate_right(tree, cursor->parent);
            }
            else if (sibling->color == RBTREE_COLOR_BLACK &&
                     (sibling->left == NULL || sibling->left->color  == RBTREE_COLOR_BLACK) &&
                     (sibling->right == NULL || sibling->right->color == RBTREE_COLOR_BLACK))
            {
                assert ((sibling->left == NULL && sibling->right == NULL) || (sibling->left != NULL && sibling->right != NULL));
                sibling->color = RBTREE_COLOR_RED;
                if (parent->color == RBTREE_COLOR_BLACK) cursor = parent;
                else
                {
                    assert (parent->color == RBTREE_COLOR_RED);
                    parent->color = RBTREE_COLOR_BLACK;
                    return rbtree_remove_free_residual_node_and_terminate(tree, to_delete);
                }
            }
            else
            {
                assert (sibling->color == RBTREE_COLOR_BLACK);
                assert ((sibling->left  != NULL && sibling->left->color  == RBTREE_COLOR_RED) ||
                        (sibling->right != NULL && sibling->right->color == RBTREE_COLOR_RED));
                if (sibling->left == NULL || sibling->left->color == RBTREE_COLOR_BLACK)
                {
                    assert (sibling->right != NULL && sibling->right->color == RBTREE_COLOR_RED);
                    rbtree_rotate_left(tree, sibling);
                    sibling = parent->left;
                    assert (sibling != NULL);
                }
                assert (sibling->left != NULL); // It was either there in the first place or it was ariginally a sibling but was moved to sibling->right due to rotation
                sibling->left->color = RBTREE_COLOR_BLACK;
                sibling->color = parent->color;
                parent->color = RBTREE_COLOR_BLACK;
                rbtree_rotate_right(tree, parent);
                return rbtree_remove_free_residual_node_and_terminate(tree, to_delete);
            }
        }
        else
        {
            assert (parent->left == cursor);
            // proceed with mirror of above
            struct RBTree* sibling = parent->right;
            assert (sibling != NULL);
            if (sibling->color == RBTREE_COLOR_RED)
            {
                sibling->color = RBTREE_COLOR_BLACK;
                parent->color = RBTREE_COLOR_RED;
                rbtree_rotate_left(tree, cursor->parent);
            }
            else if (sibling->color == RBTREE_COLOR_BLACK &&
                     (sibling->left == NULL || sibling->left->color  == RBTREE_COLOR_BLACK) &&
                     (sibling->right == NULL || sibling->right->color == RBTREE_COLOR_BLACK))
            {
                assert ((sibling->left == NULL && sibling->right == NULL) || (sibling->left != NULL && sibling->right != NULL));
                sibling->color = RBTREE_COLOR_RED;

                if (parent->color == RBTREE_COLOR_BLACK) cursor = parent;
                else
                {
                    assert (parent->color == RBTREE_COLOR_RED);
                    parent->color = RBTREE_COLOR_BLACK;
                    return rbtree_remove_free_residual_node_and_terminate(tree, to_delete);
                }
            }
            else
            {
                assert (sibling->color == RBTREE_COLOR_BLACK);
                assert ((sibling->left  != NULL && sibling->left->color  == RBTREE_COLOR_RED) ||
                        (sibling->right != NULL && sibling->right->color == RBTREE_COLOR_RED));
                if (sibling->right == NULL || sibling->right->color == RBTREE_COLOR_BLACK)
                {
                    assert (sibling->left != NULL && sibling->left->color == RBTREE_COLOR_RED);
                    rbtree_rotate_right(tree, sibling);

                    sibling = parent->right;
                    assert (sibling != NULL);
                }
                assert (sibling->right != NULL); // It was either there in the first place or it was ariginally a sibling but was moved to sibling->right due to rotation
                sibling->right->color = RBTREE_COLOR_BLACK;
                sibling->color = parent->color;
                parent->color = RBTREE_COLOR_BLACK;

                rbtree_rotate_left(tree, parent);

                return rbtree_remove_free_residual_node_and_terminate(tree, to_delete);
            }
        }
    }
}
