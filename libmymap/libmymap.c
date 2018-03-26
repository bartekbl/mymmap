#include "libmymap.h"
#include "libmymap-rbtree-dump.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "libmymap-rbtree.h"

struct MymapObjectTree
{
    struct RBTree node;
    void* addr_first;
    void* o;
    unsigned int size;
    unsigned int flags;
};

//enum RBTreeError mymap_object_insert(struct MymapObjectTree** tree,
//                                     void*                   addr_first,
//                                     unsigned int            size,
//                                     unsigned int            flags)
//{
//    void* addr_last = addr_first + size - 1;

//    assert (tree != NULL); // we must be given valid pointer to a pointer to a tree
//    assert (*tree == NULL || (*tree)->node.parent == NULL); // tree can be empty or we can operate on root but we can't start from the middle
//    assert (addr_first < addr_last);

//    struct MymapObjectTree* parent = NULL;
//    struct MymapObjectTree** pcursor = tree;
//    while (*pcursor != NULL)
//    {
//        struct MymapObjectTree* cursor = *pcursor;
//        void* cursor_addr_last = cursor->addr_first + size - 1;
//        if      (addr_last < cursor->addr_first) pcursor = (struct MymapObjectTree**)&cursor->node.left;
//        else if (addr_first > cursor_addr_last)  pcursor = (struct MymapObjectTree**)&cursor->node.right;
//        else return RBTREE_INSERT_ERROR_DUPLICATE;
//        parent = cursor;
//    }

//    struct MymapObjectTree* newval = malloc(sizeof(struct MymapObjectTree));
//    if (newval == NULL) return RBTREE_INSERT_ERROR_CANT_ALLOCATE;
//    memset(newval, 0, sizeof(struct MymapObjectTree));
//    newval->addr_first = addr_first; // left, parent, right, and color are initialized to zero which is ok (NULL, NULL, NULL and RED)
//    newval->size       = size;       // left, parent, right, and color are initialized to zero which is ok (NULL, NULL, NULL and RED)
//    newval->flags      = flags;      // left, parent, right, and color are initialized to zero which is ok (NULL, NULL, NULL and RED)

//    newval->node.parent = (struct RBTree*)parent;
//    *pcursor = newval;

//    rbtree_correct((struct RBTree**)tree, (struct RBTree*)newval);

//    return RBTREE_NO_ERROR;
//}

enum RBTreeNodeStage
{
    RBTREE_NODE_ENTERED,
    RBTREE_NODE_LEFT_INSPECTED,
    RBTREE_NODE_RIGHT_INSPECTED
};

void* mymap_mmap(struct map_t* map,
                 void*         vaddr,
                 unsigned int  size,
                 unsigned int  flags,
                 void*         o)
{
    assert (map->object_root == NULL || map->object_root->node.parent == NULL); // tree can be empty or we can operate on root but we can't start from the middle

    // first find two nodes that vaddr is in between
    // one of them will surely be descendant of the other and will have at least one NULL child
    // this will be parent node to new node (before correction) if there is enoug space
    // if we need more space we traverse the tree always keeping two adjecent nodes

    struct MymapObjectTree** pcursor = &map->object_root;
    int found_it = 0;
    void* addr_first;

    struct MymapObjectTree* left_neighbour = NULL;
    struct MymapObjectTree* right_neighbour = NULL;

    // find first pair of objects where new object can fit in between
    while (*pcursor != NULL)
    {
        struct MymapObjectTree* cursor = *pcursor;
        if (vaddr < cursor->addr_first)
        {
            right_neighbour = cursor;
            pcursor = (struct MymapObjectTree**)&cursor->node.left;
        }
        else
        {
            left_neighbour = cursor;
            pcursor = (struct MymapObjectTree**)&cursor->node.right;
        }
    }

    // try to fit it at vaddr
    assert(left_neighbour == NULL  || left_neighbour->addr_first <= vaddr);
    assert(right_neighbour == NULL || right_neighbour->addr_first > vaddr);
    if (left_neighbour == NULL || vaddr >= left_neighbour->addr_first + left_neighbour->size)
    {
        assert (right_neighbour == NULL || right_neighbour->node.left == NULL);
        void* obj_end = vaddr + size;
        if (obj_end < vaddr) return NULL; // not enough space in whole address space
        if (right_neighbour == NULL || obj_end <= right_neighbour->addr_first) // fits here!
        {
            found_it = 1;
            addr_first = vaddr;
        }
    }

    // if not enough space at vaddr, traverse the tree to find first big enough chunk of free space
    enum RBTreeNodeStage stage = RBTREE_NODE_ENTERED;
    if (!found_it) left_neighbour = right_neighbour;
    while (!found_it)
    {
        if (stage == RBTREE_NODE_ENTERED)
        {
            // traverse into left subtree.
            if (right_neighbour->node.left == NULL) stage = RBTREE_NODE_LEFT_INSPECTED;
            else
            {
                right_neighbour = (struct MymapObjectTree*)right_neighbour->node.left;
            }
        }
        if (stage == RBTREE_NODE_LEFT_INSPECTED)
        {
            assert (left_neighbour != NULL);
            void* first_left_free = left_neighbour->addr_first + left_neighbour->size;
            assert (first_left_free > left_neighbour->addr_first);
            void* obj_end = first_left_free + size;
            if (obj_end < first_left_free) return NULL; // not enough space in whole address space
            if (obj_end <= right_neighbour->addr_first) // fits here!
            {
                found_it = 1;
                addr_first = first_left_free;
                break;
            }

            left_neighbour = right_neighbour;

            if (right_neighbour->node.right == NULL) stage = RBTREE_NODE_RIGHT_INSPECTED;
            else
            {
                right_neighbour = (struct MymapObjectTree*)right_neighbour->node.right;
            }
        }
        if (stage == RBTREE_NODE_RIGHT_INSPECTED)
        {
            if (right_neighbour == map->object_root)
            {
                right_neighbour = NULL;
                break;
            }
            struct MymapObjectTree* parent = (struct MymapObjectTree*)right_neighbour->node.parent;
            // We assume we are somewhere in the middle of a tree so this assumption is safe.
            assert (parent != NULL);

            if (right_neighbour == (struct MymapObjectTree*)parent->node.right) stage = RBTREE_NODE_RIGHT_INSPECTED;
            else
            {
                assert (right_neighbour == (struct MymapObjectTree*)parent->node.left);
                stage = RBTREE_NODE_LEFT_INSPECTED;
            }
            right_neighbour = parent;
        }
    }
    // if still not found good space, try to fit behind the tree
    if (!found_it)
    {
        assert(left_neighbour != NULL);
        assert(right_neighbour == NULL);

        void* first_left_free = left_neighbour->addr_first + left_neighbour->size;
        assert (first_left_free > left_neighbour->addr_first);
        void* obj_end = first_left_free + size;
        if (obj_end < first_left_free) return NULL; // not enough space in whole address space
        found_it = 1;
        addr_first = first_left_free;
    }

    struct MymapObjectTree* parent = NULL;

    // check who is the parent of our new child
    if (left_neighbour == NULL && right_neighbour == NULL)
    {
        assert (map->object_root == NULL);
        pcursor = &map->object_root;
        parent = NULL;
    }
    if (left_neighbour == NULL && right_neighbour != NULL)
    {
        assert (right_neighbour->node.left == NULL);
        pcursor = (struct MymapObjectTree**)&right_neighbour->node.left;
        parent = right_neighbour;
    }
    if (left_neighbour != NULL && right_neighbour == NULL)
    {
        assert (left_neighbour->node.right == NULL);
        pcursor = (struct MymapObjectTree**)&left_neighbour->node.right;
        parent = left_neighbour;
    }
    if (left_neighbour != NULL && right_neighbour != NULL)
    {
        assert (right_neighbour->node.left == NULL || left_neighbour->node.right == NULL);
        assert (right_neighbour->node.left != NULL || left_neighbour->node.right != NULL);
        if (right_neighbour->node.left == NULL)
        {
            pcursor = (struct MymapObjectTree**)&right_neighbour->node.left;
            parent = right_neighbour;
        }
        else
        {
            pcursor = (struct MymapObjectTree**)&left_neighbour->node.left;
            parent = left_neighbour;
        }
    }


    struct MymapObjectTree* newval = malloc(sizeof(struct MymapObjectTree));
    if (newval == NULL) return NULL;
    memset(newval, 0, sizeof(struct MymapObjectTree));
    newval->addr_first = addr_first; // left, parent, right, and color are initialized to zero which is ok (NULL, NULL, NULL and RED)
    newval->size       = size;       // left, parent, right, and color are initialized to zero which is ok (NULL, NULL, NULL and RED)
    newval->flags      = flags;      // left, parent, right, and color are initialized to zero which is ok (NULL, NULL, NULL and RED)
    newval->o          = o;          // left, parent, right, and color are initialized to zero which is ok (NULL, NULL, NULL and RED)

    newval->node.parent = (struct RBTree*)parent;
    *pcursor = newval;

    rbtree_correct((struct RBTree**)&map->object_root, (struct RBTree*)newval);

    return newval->addr_first;
}

void mymap_munmap(struct map_t* map,
                  void*         vaddr)
{
    assert (map->object_root == NULL || map->object_root->node.parent == NULL); // tree can be empty or we can operate on root but we can't start from the middle

    struct MymapObjectTree* cursor = map->object_root;

    while (1)
    {
        if (cursor == NULL) return; // not found

        assert (cursor->addr_first + cursor->size > cursor->addr_first);

        if (vaddr >= cursor->addr_first && vaddr < (cursor->addr_first + cursor->size)) break; // Found it!

        if (vaddr < cursor->addr_first) cursor = (struct MymapObjectTree*)cursor->node.left;
        else                            cursor = (struct MymapObjectTree*)cursor->node.right;
    }

    rbtree_remove((struct RBTree**)&map->object_root, (struct RBTree*)cursor);

    free(cursor);
}

int mymap_init(struct map_t *map)
{
    map->object_root = NULL;
    return 0;
}

void print_node(struct RBTree* node)
{
    struct MymapObjectTree* object = (struct MymapObjectTree*)node;
    printf("o=%p, addr=%p, size=%X, flags=%X", object->o, object->addr_first, object->size, object->flags);
}

int mymap_dump(struct map_t *map)
{
    rbtree_dump(map->object_root, print_node);
    return 0;
}
