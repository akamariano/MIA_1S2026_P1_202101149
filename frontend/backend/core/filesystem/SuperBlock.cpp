#include "SuperBlock.h"
#include "Inode.h"
#include <cmath>

SuperBlock createSuperBlock(int partition_start, int partition_size) {

    SuperBlock sb;

    int inode_size = sizeof(Inode);
    int block_size = 64; // todos los bloques son 64 bytes

    // Fórmula para calcular n
    double n = (double)(partition_size - sizeof(SuperBlock)) /
               (1 + 3 + inode_size + (3 * block_size));

    n = floor(n);

    int total_inodes = (int)n;
    int total_blocks = 3 * total_inodes;

    sb.s_filesystem_type = 2;
    sb.s_inodes_count = total_inodes;
    sb.s_blocks_count = total_blocks;

    sb.s_free_inodes_count = total_inodes - 2; // root y users.txt
    sb.s_free_blocks_count = total_blocks - 2;

    sb.s_mtime = time(nullptr);
    sb.s_umtime = 0;
    sb.s_mnt_count = 1;

    sb.s_magic = 0xEF53;
    sb.s_inode_size = inode_size;
    sb.s_block_size = block_size;

    sb.s_first_ino = 2;
    sb.s_first_blo = 2;

    // Offsets
    sb.s_bm_inode_start = partition_start + sizeof(SuperBlock);
    sb.s_bm_block_start = sb.s_bm_inode_start + total_inodes;
    sb.s_inode_start = sb.s_bm_block_start + total_blocks;
    sb.s_block_start = sb.s_inode_start + (total_inodes * inode_size);

    return sb;
}
