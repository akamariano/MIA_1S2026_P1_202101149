#include <iostream>
#include <vector>
#include <cmath>
#include <ctime>
#include <cstring>
#include "mkfs.h"
#include "../mount/mount_manager.h"
#include "../core/filesystem/SuperBlock.h"
#include "../core/filesystem/Inode.h"
#include "../core/filesystem/Blocks.h"

using namespace std;

// ================= BITMAP =================

void setBit(vector<unsigned char>& bitmap, int index) {
    int byteIndex = index / 8;
    int bitIndex = index % 8;
    bitmap[byteIndex] |= (1 << bitIndex);
}

// ================= MKFS =================

void Mkfs::execute(string id) {

    MountedPartition* part = MountManager::getMountedById(id);

    if (!part) {
        cout << "ERROR: ID no montado\n";
        return;
    }

    FILE* file = fopen(part->path.c_str(), "rb+");
    if (!file) {
        cout << "ERROR: No se pudo abrir el disco\n";
        return;
    }

    int partitionStart = part->start;
    int partitionSize  = part->size;

    // ================= CALCULO EXT2 (SOLO ESTRUCTURAS) =================

    double denominator =
        sizeof(Inode)
        + 3*sizeof(FileBlock)
        + (1.0/8.0)
        + (3.0/8.0);

    int n = floor((partitionSize - sizeof(SuperBlock)) / denominator);

    if (n <= 0) {
        cout << "ERROR: Partición demasiado pequeña.\n";
        fclose(file);
        return;
    }

    int numInodes = n;
    int numBlocks = 3 * n;

    // ================= SUPERBLOCK =================

    SuperBlock sb = {};

    sb.s_filesystem_type = 2;
    sb.s_inodes_count = numInodes;
    sb.s_blocks_count = numBlocks;

    sb.s_free_inodes_count = numInodes - 1; // solo root usado
    sb.s_free_blocks_count = numBlocks - 1; // solo bloque root usado

    sb.s_mtime = time(nullptr);
    sb.s_umtime = time(nullptr);
    sb.s_mnt_count = 1;
    sb.s_magic = 0xEF53;

    sb.s_inode_size = sizeof(Inode);
    sb.s_block_size = sizeof(FileBlock);

    sb.s_first_ino = 1;
    sb.s_first_blo = 1;

    // ================= POSICIONES =================

    int bytesInodeBitmap = ceil(numInodes / 8.0);
    int bytesBlockBitmap = ceil(numBlocks / 8.0);

    sb.s_bm_inode_start = partitionStart + sizeof(SuperBlock);
    sb.s_bm_block_start = sb.s_bm_inode_start + bytesInodeBitmap;
    sb.s_inode_start = sb.s_bm_block_start + bytesBlockBitmap;
    sb.s_block_start = sb.s_inode_start + (numInodes * sizeof(Inode));

    // ================= BITMAPS =================

    vector<unsigned char> bm_inode(bytesInodeBitmap, 0);
    vector<unsigned char> bm_block(bytesBlockBitmap, 0);

    // solo root
    setBit(bm_inode, 0);
    setBit(bm_block, 0);

    // ================= INODO ROOT =================

    Inode root = {};

    root.i_uid = 1;
    root.i_gid = 1;
    root.i_size = 0;
    root.i_type = '0';  // carpeta
    root.i_perm = 664;
    root.i_block[0] = 0;

    root.i_atime = root.i_ctime = root.i_mtime = time(nullptr);

    // ================= BLOQUE ROOT =================

    DirectoryBlock rootBlock = {};

    strcpy(rootBlock.b_content[0].b_name, ".");
    rootBlock.b_content[0].b_inodo = 0;

    strcpy(rootBlock.b_content[1].b_name, "..");
    rootBlock.b_content[1].b_inodo = 0;

    rootBlock.b_content[2].b_inodo = -1;
    rootBlock.b_content[3].b_inodo = -1;

    // ================= ESCRITURA =================

    fseek(file, partitionStart, SEEK_SET);
    fwrite(&sb, sizeof(SuperBlock), 1, file);

    fseek(file, sb.s_bm_inode_start, SEEK_SET);
    fwrite(bm_inode.data(), 1, bytesInodeBitmap, file);

    fseek(file, sb.s_bm_block_start, SEEK_SET);
    fwrite(bm_block.data(), 1, bytesBlockBitmap, file);

    fseek(file, sb.s_inode_start, SEEK_SET);
    fwrite(&root, sizeof(Inode), 1, file);

    fseek(file, sb.s_block_start, SEEK_SET);
    fwrite(&rootBlock, sizeof(DirectoryBlock), 1, file);

    fclose(file);

    cout << "MKFS realizado correctamente (EXT2 base sin users).\n";
}
