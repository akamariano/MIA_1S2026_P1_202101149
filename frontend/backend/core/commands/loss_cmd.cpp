#include "loss_cmd.h"
#include "../mount/mount_manager.h"
#include "../filesystem/session_manager.h"
#include "../core/filesystem/EXT2Utils.h"
#include "../core/filesystem/SuperBlock.h"
#include "../core/filesystem/Inode.h"
#include "../core/filesystem/Blocks.h"
#include <iostream>
#include <cstring>
#include <vector>
using namespace std;

void LossCmd::execute(const string& id) {

    if (!SessionManager::isActive()) {
        cout << "ERROR: No hay sesión activa\n"; return;
    }
    if (!SessionManager::isRoot()) {
        cout << "ERROR: Solo root puede ejecutar loss\n"; return;
    }

    MountedPartition* part = MountManager::getMountedById(id);
    if (!part) {
        cout << "ERROR: ID '" << id << "' no está montado\n"; return;
    }

    FILE* disk = fopen(part->path.c_str(), "rb+");
    if (!disk) {
        cout << "ERROR: No se pudo abrir el disco\n"; return;
    }

    SuperBlock sb;
    readSuperBlock(disk, part->start, sb);

    // Solo EXT3 tiene journaling
    if (sb.s_filesystem_type != 3) {
        cout << "ERROR: La partición no es EXT3\n";
        fclose(disk); return;
    }

    long long bm_inode_size = sb.s_inodes_count;
    long long bm_block_size = sb.s_blocks_count;
    long long inode_size    = (long long)sb.s_inodes_count * sb.s_inode_size;
    long long block_size    = (long long)sb.s_blocks_count * sb.s_block_size;

    // Limpiar bitmap de inodos con \0
    vector<char> zeros_bm_inode(bm_inode_size, '\0');
    fseek(disk, sb.s_bm_inode_start, SEEK_SET);
    fwrite(zeros_bm_inode.data(), 1, bm_inode_size, disk);

    // Limpiar bitmap de bloques con \0
    vector<char> zeros_bm_block(bm_block_size, '\0');
    fseek(disk, sb.s_bm_block_start, SEEK_SET);
    fwrite(zeros_bm_block.data(), 1, bm_block_size, disk);

    // Limpiar área de inodos con \0
    vector<char> zeros_inodes(inode_size, '\0');
    fseek(disk, sb.s_inode_start, SEEK_SET);
    fwrite(zeros_inodes.data(), 1, inode_size, disk);

    // Limpiar área de bloques con \0
    vector<char> zeros_blocks(block_size, '\0');
    fseek(disk, sb.s_block_start, SEEK_SET);
    fwrite(zeros_blocks.data(), 1, block_size, disk);

    fclose(disk);

    cout << "OK: Sistema de archivos EXT3 en '" << id << "' simulado como perdido\n";
    cout << "    Bitmap inodos  : limpiado (" << bm_inode_size << " bytes)\n";
    cout << "    Bitmap bloques : limpiado (" << bm_block_size << " bytes)\n";
    cout << "    Area inodos    : limpiada (" << inode_size    << " bytes)\n";
    cout << "    Area bloques   : limpiada (" << block_size    << " bytes)\n";
}