#ifndef BLOCKS_H
#define BLOCKS_H

#include <cstring>

#pragma pack(push, 1)

struct Content {
    char b_name[12];
    int  b_inodo;
};

struct DirectoryBlock {
    Content b_content[4];
};

struct FileBlock {
    char b_content[64];
};

struct PointerBlock {
    int b_pointers[16];
};

#pragma pack(pop)

#endif