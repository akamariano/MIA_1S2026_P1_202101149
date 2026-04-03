#include "chmod_cmd.h"
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

static void chmodRecursive(FILE* disk, SuperBlock& sb,
                            int inodeNum, int newPerm,
                            int currentUid, bool isRoot) {
    Inode inode;
    readInode(disk, sb, inodeNum, inode);

    // Solo root o propietario puede cambiar permisos
    if (isRoot || inode.i_uid == currentUid) {
        inode.i_perm = newPerm;
        writeInode(disk, sb, inodeNum, inode);
    }

    if (inode.i_type != '0') return;

    for (int b = 0; b < 12; b++) {
        if (inode.i_block[b] == -1) break;
        DirectoryBlock db;
        memset(&db, 0, sizeof(DirectoryBlock));
        readBlock(disk, sb, inode.i_block[b], &db);
        for (int e = 0; e < 4; e++) {
            if (db.b_content[e].b_inodo == -1) continue;
            string ename(db.b_content[e].b_name,
                         strnlen(db.b_content[e].b_name, 12));
            if (ename == "." || ename == "..") continue;
            chmodRecursive(disk, sb, db.b_content[e].b_inodo,
                           newPerm, currentUid, isRoot);
        }
    }
}

void ChmodCmd::execute(const string& path,
                        const string& ugo,
                        bool recursive) {

    if (!SessionManager::isActive()) {
        cout << "ERROR: No hay sesión activa\n"; return;
    }

    // Validar ugo — debe ser exactamente 3 dígitos 0-7
    if (ugo.size() != 3) {
        cout << "ERROR: -ugo debe tener exactamente 3 dígitos (ej: 664)\n";
        return;
    }
    for (char c : ugo) {
        if (c < '0' || c > '7') {
            cout << "ERROR: Cada dígito de -ugo debe estar entre 0 y 7\n";
            return;
        }
    }
    int newPerm = (ugo[0]-'0')*100 + (ugo[1]-'0')*10 + (ugo[2]-'0');

    MountedPartition* part = MountManager::getMountedById(
                                SessionManager::get().partitionId);
    if (!part) { cout << "ERROR: Partición no encontrada\n"; return; }

    FILE* disk = fopen(part->path.c_str(), "rb+");
    if (!disk) { cout << "ERROR: No se pudo abrir el disco\n"; return; }

    SuperBlock sb;
    readSuperBlock(disk, part->start, sb);

    int targetInode = resolvePath(disk, sb, path);
    if (targetInode == -1) {
        cout << "ERROR: La ruta '" << path << "' no existe\n";
        fclose(disk); return;
    }

    bool isRoot    = SessionManager::isRoot();
    int currentUid = SessionManager::get().uid;

    // Verificar que el usuario es propietario (a menos que sea root)
    Inode targetInodeData;
    readInode(disk, sb, targetInode, targetInodeData);
    if (!isRoot && targetInodeData.i_uid != currentUid) {
        cout << "ERROR: Solo puede cambiar permisos de sus propios archivos\n";
        fclose(disk); return;
    }

    if (recursive) {
        chmodRecursive(disk, sb, targetInode, newPerm, currentUid, isRoot);
    } else {
        targetInodeData.i_perm = newPerm;
        writeInode(disk, sb, targetInode, targetInodeData);
    }

    // Journaling
    JournalManager::write(disk, sb, part->start, "chmod", path, ugo);

    writeSuperBlock(disk, part->start, sb);
    fclose(disk);

    cout << "OK: Permisos de '" << path << "' cambiados a " << ugo
         << (recursive ? " (recursivo)" : "") << "\n";
}