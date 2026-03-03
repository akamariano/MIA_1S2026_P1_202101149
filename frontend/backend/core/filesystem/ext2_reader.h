#ifndef EXT2_READER_H
#define EXT2_READER_H

#include <string>
#include <cstdio>
#include "../core/filesystem/SuperBlock.h"
#include "../core/filesystem/Inode.h"
#include "../core/filesystem/Blocks.h"

class EXT2Reader {
public:
    // Lee y retorna el contenido de un archivo como string
    static std::string readFile(FILE* disk, SuperBlock& sb,
                                long long partStart,
                                const std::string& path);
};
#endif