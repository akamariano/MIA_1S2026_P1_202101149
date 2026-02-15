#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#include <ctime>

#pragma pack(push, 1) // evita padding
struct SuperBlock {

    int s_filesystem_type;      // 2 = EXT2
    int s_inodes_count;         // Total inodos
    int s_blocks_count;         // Total bloques
    int s_free_blocks_count;    // Bloques libres
    int s_free_inodes_count;    // Inodos libres

    time_t s_mtime;             // Último montaje
    time_t s_umtime;            // Último desmontaje
    int s_mnt_count;            // Cantidad de montajes

    int s_magic;                // 0xEF53
    int s_inode_size;              // Tamaño del inodo
    int s_block_size;              // Tamaño del bloque

    int s_first_ino;            // Primer inodo libre
    int s_first_blo;            // Primer bloque libre

    int s_bm_inode_start;       // Inicio bitmap inodos
    int s_bm_block_start;       // Inicio bitmap bloques
    int s_inode_start;          // Inicio tabla inodos
    int s_block_start;          // Inicio tabla bloques

};

#pragma pack(pop)

#endif
