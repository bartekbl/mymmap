#ifndef RBTREEINTERNAL_H
#define RBTREEINTERNAL_H

struct RBTree* rbtree_rotate_left(struct RBTree** tree, struct RBTree* parent);
struct RBTree* rbtree_rotate_right(struct RBTree** tree, struct RBTree* parent);
struct RBTree** rbtree_get_pointer_to_parent_pointer_to_node(struct RBTree** tree, struct RBTree* node);

#endif // RBTREEINTERNAL_H
