#ifndef RBTREEDUMP_H
#define RBTREEDUMP_H

#include "libmymap-rbtree.h"

typedef void (*PrintFunction)(const struct RBTree* node);

void rbtree_dump(const struct RBTree* tree, PrintFunction print);

#endif // RBTREEDUMP_H
