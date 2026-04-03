#include "rename_cmd.h"
#include "../filesystem/session_manager.h"
#include "../filesystem/permissions.h"
#include "../filesystem/journal_manager.h"
#include "../filesystem/EXT2Utils.h"
#include "../mount/mount_manager.h"
#include "../core/filesystem/SuperBlock.h"
#include "../core/filesystem/Inode.h"
#include "../core/filesystem/Blocks.h"
#include <iostream>
#include <cstring>
using namespace std;

void RenameCmd::execute(const string& path, const string& newName) {

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

    // Verificar que el path existe
    int targetInode = resolvePath(disk, sb, path);
    if (targetInode == -1) {
        cout << "ERROR: La ruta '" << path << "' no existe\n";
        fclose(disk); return;
    }

    // Verificar permisos de escritura
    Inode targetInodeData;
    readInode(disk, sb, targetInode, targetInodeData);
    if (!Permissions::canWrite(targetInodeData)) {
        cout << "ERROR: No tiene permiso de escritura sobre '" << path << "'\n";
        fclose(disk); return;
    }

    // Obtener carpeta padre
    string parentPath = "/";
    size_t lastSlash = path.rfind('/');
    if (lastSlash != string::npos && lastSlash > 0)
        parentPath = path.substr(0, lastSlash);

    int parentInode = resolvePath(disk, sb, parentPath);
    if (parentInode == -1) {
        cout << "ERROR: No se encontró carpeta padre\n";
        fclose(disk); return;
    }

    // Verificar que el nuevo nombre no existe en el padre
    int existing = findInDirectory(disk, sb, parentInode, newName);
    if (existing != -1) {
        cout << "ERROR: Ya existe '" << newName << "' en el directorio\n";
        fclose(disk); return;
    }

    // Buscar y renombrar la entrada en el directorio padre
    Inode parent;
    readInode(disk, sb, parentInode, parent);
    bool renamed = false;

    for (int b = 0; b < 12 && !renamed; b++) {
        if (parent.i_block[b] == -1) break;
        DirectoryBlock db;
        readBlock(disk, sb, parent.i_block[b], &db);
        for (int e = 0; e < 4; e++) {
            if (db.b_content[e].b_inodo == targetInode) {
                memset(db.b_content[e].b_name, 0, 12);
                strncpy(db.b_content[e].b_name, newName.c_str(), 11);
                writeBlock(disk, sb, parent.i_block[b], &db);
                renamed = true;
                break;
            }
        }
    }

    if (!renamed) {
        cout << "ERROR: No se pudo renombrar\n";
        fclose(disk); return;
    }

    // Journaling
    JournalManager::write(disk, sb, part->start, "rename",
                          path, newName);

    writeSuperBlock(disk, part->start, sb);
    fclose(disk);

    cout << "OK: '" << path << "' renombrado a '" << newName << "'\n";
}