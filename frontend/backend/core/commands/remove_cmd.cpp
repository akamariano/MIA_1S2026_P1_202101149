#include "remove_cmd.h"
#include "../filesystem/session_manager.h"
#include "../filesystem/permissions.h"
#include "../filesystem/journal_manager.h"
#include "../filesystem/EXT2Utils.h"
#include "../filesystem/ext2_writer.h"
#include "../mount/mount_manager.h"
#include "../core/filesystem/SuperBlock.h"
#include "../core/filesystem/Inode.h"
#include "../core/filesystem/Blocks.h"
#include <iostream>
#include <cstring>
using namespace std;

// Libera recursivamente todos los bloques e inodos de un inodo
static void freeInodeRecursive(FILE* disk, SuperBlock& sb, int inodeNum) {
    if (inodeNum == -1) return;

    Inode inode;
    readInode(disk, sb, inodeNum, inode);

    // Si es carpeta, liberar hijos recursivamente
    if (inode.i_type == '0') {
        for (int b = 0; b < 12; b++) {
            if (inode.i_block[b] == -1) break;
            DirectoryBlock db;
            memset(&db, 0, sizeof(DirectoryBlock));
            readBlock(disk, sb, inode.i_block[b], &db);
            for (int e = 0; e < 4; e++) {
                if (db.b_content[e].b_inodo == -1) continue;
                string name(db.b_content[e].b_name,
                            strnlen(db.b_content[e].b_name, 12));
                if (name == "." || name == "..") continue;
                freeInodeRecursive(disk, sb, db.b_content[e].b_inodo);
            }
            // Liberar bloque de directorio
            writeBitmapBlock(disk, sb, inode.i_block[b], 0);
            sb.s_free_blocks_count++;
        }
    } else {
        // Liberar bloques de archivo
        for (int b = 0; b < 12; b++) {
            if (inode.i_block[b] == -1) break;
            writeBitmapBlock(disk, sb, inode.i_block[b], 0);
            sb.s_free_blocks_count++;
        }
        // Bloque indirecto
        if (inode.i_block[12] != -1) {
            PointerBlock pb;
            readBlock(disk, sb, inode.i_block[12], &pb);
            for (int i = 0; i < 16; i++) {
                if (pb.b_pointers[i] == -1) break;
                writeBitmapBlock(disk, sb, pb.b_pointers[i], 0);
                sb.s_free_blocks_count++;
            }
            writeBitmapBlock(disk, sb, inode.i_block[12], 0);
            sb.s_free_blocks_count++;
        }
    }

    // Liberar el inodo
    writeBitmapInode(disk, sb, inodeNum, 0);
    sb.s_free_inodes_count++;
}

// Elimina la entrada de un inodo en el directorio padre
static bool removeEntryFromParent(FILE* disk, SuperBlock& sb,
                                   int parentInode, int targetInode) {
    Inode parent;
    readInode(disk, sb, parentInode, parent);

    for (int b = 0; b < 12; b++) {
        if (parent.i_block[b] == -1) break;
        DirectoryBlock db;
        readBlock(disk, sb, parent.i_block[b], &db);
        for (int e = 0; e < 4; e++) {
            if (db.b_content[e].b_inodo == targetInode) {
                db.b_content[e].b_inodo = -1;
                memset(db.b_content[e].b_name, 0, 12);
                writeBlock(disk, sb, parent.i_block[b], &db);
                return true;
            }
        }
    }
    return false;
}

void RemoveCmd::execute(const string& path) {

    if (!SessionManager::isActive()) {
        cout << "ERROR: No hay sesión activa\n"; return;
    }

    MountedPartition* part = MountManager::getMountedById(
                                SessionManager::get().partitionId);
    if (!part) { cout << "ERROR: Partición no encontrada\n"; return; }

    FILE* disk = fopen(part->path.c_str(), "rb+");
    if (!disk) { cout << "ERROR: No se pudo abrir el disco\n"; return; }

    SuperBlock sb;
    readSuperBlock(disk, part->start, sb);

    // Resolver el inodo del path
    int targetInode = resolvePath(disk, sb, path);
    if (targetInode == -1) {
        cout << "ERROR: La ruta '" << path << "' no existe\n";
        fclose(disk); return;
    }

    // Verificar permisos de escritura
    Inode targetInodeData;
    readInode(disk, sb, targetInode, targetInodeData);
    if (!Permissions::canWrite(targetInodeData)) {
        cout << "ERROR: No tiene permiso para eliminar '" << path << "'\n";
        fclose(disk); return;
    }

    // Encontrar el inodo padre
    // Separar el path en padre y nombre
    string parentPath = "/";
    string name = path;
    size_t lastSlash = path.rfind('/');
    if (lastSlash != string::npos && lastSlash > 0) {
        parentPath = path.substr(0, lastSlash);
        name = path.substr(lastSlash + 1);
    } else if (lastSlash == 0) {
        parentPath = "/";
        name = path.substr(1);
    }

    int parentInode = resolvePath(disk, sb, parentPath);
    if (parentInode == -1) {
        cout << "ERROR: No se encontró carpeta padre\n";
        fclose(disk); return;
    }

    // Liberar recursivamente
    freeInodeRecursive(disk, sb, targetInode);

    // Eliminar entrada del padre
    removeEntryFromParent(disk, sb, parentInode, targetInode);

    // Journaling
    JournalManager::write(disk, sb, part->start, "remove", path);

    writeSuperBlock(disk, part->start, sb);
    fclose(disk);

    cout << "OK: '" << path << "' eliminado correctamente\n";
}