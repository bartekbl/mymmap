#include "libmymap-rbtree-dump.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

enum RBTreeNodeStage
{
    RBTREE_NODE_ENTERED,
    RBTREE_NODE_LEFT_INSPECTED,
    RBTREE_NODE_RIGHT_INSPECTED
};

struct RBTreeDumpTraverseAlgorithm
{
    const struct RBTree* top;
    const struct RBTree* cursor;
    PrintFunction        print;
    enum RBTreeNodeStage stage;
    int                  level;

    // each bit denotes a turn at each level
    // 0 means left-turn and 1 means right-turn
    // it allows us to trace only up to 64 levels of a tree but it is already more than human can see
    u_int64_t            turns_map;
};

static void rbtree_dump_draw_connection(struct RBTreeDumpTraverseAlgorithm* a)
{
    int this_turn = a->turns_map & 1;
    for (int i = 0; i < a->level - 1; i++)
    {
        int next_turn = 1 & (a->turns_map >> (i + 1));
        if (next_turn == this_turn) printf("  ");// fflush(stdout);
        else                        printf("│ ");// fflush(stdout);
        this_turn = next_turn;
    }
    if (a->level > 0)
    {
        if (this_turn) printf("└─");// fflush(stdout);
        else           printf("┌─");// fflush(stdout);
    }

}

static void rbtree_dump_print_current_node(struct RBTreeDumpTraverseAlgorithm* a)
{
    rbtree_dump_draw_connection(a);
//    printf("%c:%d\n", (a->cursor->color ? 'B' : 'R'), a->cursor->value);// fflush(stdout);
    printf("%c:", (a->cursor->color ? 'B' : 'R'));
    a->print(a->cursor);
    printf("\n");
}

static void rbtree_dump_traverse_left(struct RBTreeDumpTraverseAlgorithm* a)
{
    a->turns_map &= ~(((u_int64_t)1) << a->level); // clear bit to indicate left-turn
    a->level++;
    a->cursor = a->cursor->left;
//    a->stage = RBTREE_NODE_ENTERED; // Not needed but left here as a reminder.
}

static void rbtree_dump_traverse_right(struct RBTreeDumpTraverseAlgorithm* a)
{
    a->turns_map |= ((u_int64_t)1) << a->level; // set bit to indicate right-turn
    a->level++;
    a->cursor = a->cursor->right;
    a->stage = RBTREE_NODE_ENTERED;
}

static void rbtree_dump_traverse_up(struct RBTreeDumpTraverseAlgorithm* a)
{
    struct RBTree* parent = a->cursor->parent;
    // We assume we are somewhere in the middle of a tree so this assumption is safe.
    assert (parent != NULL);

    a->level--;
    if (a->cursor == parent->right) a->stage = RBTREE_NODE_RIGHT_INSPECTED;
    else
    {
        // There are at most two subtrees: left and right so if we are not returning from right we must return from left. Hence this assumption is safe.
        assert (a->cursor == parent->left);
        a->stage = RBTREE_NODE_LEFT_INSPECTED;
    }
    a->cursor = parent;
}

void rbtree_dump(const struct RBTree* tree, PrintFunction print)
{
    if (tree == NULL)
    {
        printf("tree NULL\n");// fflush(stdout);
        return;
    }

    struct RBTreeDumpTraverseAlgorithm a = { tree, tree, print, RBTREE_NODE_ENTERED, 0, 0 };

    while (1)
    {
        if (a.stage == RBTREE_NODE_ENTERED)
        {
            // Dump left subtree.
            if (a.cursor->left == NULL) a.stage = RBTREE_NODE_LEFT_INSPECTED;
            else                        rbtree_dump_traverse_left(&a);
        }
        if (a.stage == RBTREE_NODE_LEFT_INSPECTED)
        {
            rbtree_dump_print_current_node(&a);

            if (a.cursor->right == NULL) a.stage = RBTREE_NODE_RIGHT_INSPECTED;
            else                         rbtree_dump_traverse_right(&a);
        }
        if (a.stage == RBTREE_NODE_RIGHT_INSPECTED)
        {
//            if (a.cursor == a.top) return;        // In this case break and return are equivalent but if we wanted e.g. write the summary of a tree under said tree than return would not be good
            if (a.cursor == a.top) break;         // I'm not sure which condition is better (this or the one below) but I'm leaning towards this one.
//            if (a.cursor->parent == NULL) return; // Because this one will dump too much if a subtree of a tree is given (tree->parent != NULL) plus it is propably slower
            rbtree_dump_traverse_up(&a);
        }
    }
}
