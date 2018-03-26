#ifndef LIBMYMMAP_H
#define LIBMYMMAP_H

#include "libmymap-rbtree.h"

struct map_t
{
    struct MymapObjectTree* object_root;
};

void* mymap_mmap(struct map_t* map,
                 void*         vaddr,
                 unsigned int  size,
                 unsigned int  flags,
                 void*         o);

void  mymap_munmap(struct map_t* map,
                   void*         vaddr);

int   mymap_init(struct map_t* map);

int   mymap_dump(struct map_t* map);

#endif // LIBMYMMAP_H
