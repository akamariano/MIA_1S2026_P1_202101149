#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#include <ctime>

#pragma pack(push, 1)

struct SuperBlock {
    int    s_filesystem_type;
    int    s_inodes_count;
    int    s_blocks_count;
    int    s_free_blocks_count;
    int    s_free_inodes_count;

    time_t s_mtime;
    time_t s_umtime;
    int    s_mnt_count;
    int    s_magic;
    int    s_inode_size;
    int    s_block_size;

    int    s_first_ino;
    int    s_first_blo;

    //  long long para soportar discos grandes
    long long s_bm_inode_start;
    long long s_bm_block_start;
    long long s_inode_start;
    long long s_block_start;
};

#pragma pack(pop)

#endif