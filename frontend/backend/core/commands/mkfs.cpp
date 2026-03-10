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
#include "../core/filesystem/ext2_writer.h"
using namespace std;

void Mkfs::execute(string id) {

    // Obtener la partición montada con ese ID
    MountedPartition* part = MountManager::getMountedById(id);
    if (!part) {
        cout << "ERROR: ID no montado\n";
        return;
    }

    // Abrir el archivo de disco para lectura/escritura
    FILE* file = fopen(part->path.c_str(), "rb+");
    if (!file) {
        cout << "ERROR: No se pudo abrir el disco\n";
        return;
    }

    long long partitionStart = part->start;
    long long partitionSize  = part->size;

    // Calcular número de inodos usando la fórmula:
    // n = (tamaño_partición - SB) / (1 + 3 + sizeof(Inode) + 3*sizeof(FileBlock))
    double denominator = 1.0 + 3.0
                       + (double)sizeof(Inode)
                       + 3.0 * (double)sizeof(FileBlock);

    int n = (int)floor((partitionSize - (double)sizeof(SuperBlock)) / denominator);

    // Verificar que la partición sea lo suficientemente grande
    if (n <= 0) {
        cout << "ERROR: Partición demasiado pequeña\n";
        fclose(file);
        return;
    }

    // Definir cantidades: n inodos, 3n bloques
    int numInodes = n;
    int numBlocks = 3 * n;

    // Tamaños en bytes
    int bytesInodeBitmap = numInodes;
    int bytesBlockBitmap = numBlocks;

    // Calcular posiciones en disco
    long long bm_inode_start = partitionStart + sizeof(SuperBlock);
    long long bm_block_start = bm_inode_start + bytesInodeBitmap;
    long long inode_start    = bm_block_start + bytesBlockBitmap;
    long long block_start    = inode_start    + (long long)(numInodes * sizeof(Inode));

    // Crear y configurar el SuperBloque
    SuperBlock sb;
    memset(&sb, 0, sizeof(SuperBlock));
    sb.s_filesystem_type   = 2;
    sb.s_inodes_count      = numInodes;
    sb.s_blocks_count      = numBlocks;
    sb.s_free_inodes_count = numInodes - 1;
    sb.s_free_blocks_count = numBlocks - 1;
    sb.s_mtime             = time(nullptr);
    sb.s_umtime            = 0;
    sb.s_mnt_count         = 1;
    sb.s_magic             = 0xEF53;
    sb.s_inode_size        = sizeof(Inode);
    sb.s_block_size        = sizeof(FileBlock);
    sb.s_first_ino         = 1;
    sb.s_first_blo         = 1;
    sb.s_bm_inode_start    = bm_inode_start;
    sb.s_bm_block_start    = bm_block_start;
    sb.s_inode_start       = inode_start;
    sb.s_block_start       = block_start;

    // Inicializar mapas de bits (todos los bits en 0 = libres)
    vector<unsigned char> bm_inode(bytesInodeBitmap, 0);
    vector<unsigned char> bm_block(bytesBlockBitmap, 0);
    bm_inode[0] = 1;  // Inode 0 usado (root)
    bm_block[0] = 1;  // Bloque 0 usado (contenido de root)

    // Crear el inode raíz
    Inode root;
    memset(&root, 0, sizeof(Inode));
    root.i_uid   = 1;
    root.i_gid   = 1;
    root.i_size  = sizeof(DirectoryBlock);
    root.i_type  = '0';
    root.i_perm  = 664;
    root.i_atime = root.i_ctime = root.i_mtime = time(nullptr);
    for (int i = 0; i < 15; i++) root.i_block[i] = -1;
    root.i_block[0] = 0;

    // Crear bloque del directorio raíz con . y ..
    DirectoryBlock rootBlock;
    memset(&rootBlock, 0, sizeof(DirectoryBlock));
    strncpy(rootBlock.b_content[0].b_name, ".",  11);
    rootBlock.b_content[0].b_inodo = 0;
    strncpy(rootBlock.b_content[1].b_name, "..", 11);
    rootBlock.b_content[1].b_inodo = 0;
    rootBlock.b_content[2].b_inodo = -1;
    rootBlock.b_content[3].b_inodo = -1;

    // Escribir en disco comenzando en la partición
    fseek(file, partitionStart, SEEK_SET);
    fwrite(&sb, sizeof(SuperBlock), 1, file);

    fseek(file, bm_inode_start, SEEK_SET);
    fwrite(bm_inode.data(), 1, bytesInodeBitmap, file);

    fseek(file, bm_block_start, SEEK_SET);
    fwrite(bm_block.data(), 1, bytesBlockBitmap, file);

    fseek(file, inode_start, SEEK_SET);
    fwrite(&root, sizeof(Inode), 1, file);

    fseek(file, block_start, SEEK_SET);
    fwrite(&rootBlock, sizeof(DirectoryBlock), 1, file);

    // ===== CREAR USERS.TXT =====
    EXT2Writer::createUsersFile(file, sb, partitionStart);

    //UN SOLO fclose
    fclose(file);

    cout << "OK: MKFS realizado correctamente\n";
    cout << "    Inodos : " << numInodes << "\n";
    cout << "    Bloques: " << numBlocks << "\n";
    cout << "    Inicio superblock : " << partitionStart << "\n";
    cout << "    Inicio bm_inodos  : " << bm_inode_start << "\n";
    cout << "    Inicio bm_bloques : " << bm_block_start << "\n";
    cout << "    Inicio inodos     : " << inode_start    << "\n";
    cout << "    Inicio bloques    : " << block_start    << "\n";
}