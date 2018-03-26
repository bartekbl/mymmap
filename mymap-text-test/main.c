#include <stdio.h>

#include "libmymap.h"

int main()
{
    struct map_t map;
    mymap_init(&map);
    mymap_mmap(&map, (void*)0x20110000, 0x100, 0xDEAD0001, (void*)0xFEED0001);
    mymap_mmap(&map, (void*)0x20110200, 0x100, 0xDEAD0002, (void*)0xFEED0002);
    mymap_mmap(&map, (void*)0x20110000, 0x300, 0xDEAD0003, (void*)0xFEED0003);
    mymap_mmap(&map, (void*)0x10110000, 0x300, 0xDEAD0004, (void*)0xFEED0004);
    mymap_dump(&map);
    return 0;
}
