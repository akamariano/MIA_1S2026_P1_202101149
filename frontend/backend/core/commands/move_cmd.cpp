#include "move_cmd.h"
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

// Elimina entrada de un directorio padre por nombre
static bool removeEntryByName(FILE* disk, SuperBlock& sb,
                               int parentInode, const string& name) {
    Inode parent;
    readInode(disk, sb, parentInode, parent);

    for (int b = 0; b < 12; b++) {
        if (parent.i_block[b] == -1) break;
        DirectoryBlock db;
        readBlock(disk, sb, parent.i_block[b], &db);
        for (int e = 0; e < 4; e++) {
            string ename(db.b_content[e].b_name,
                         strnlen(db.b_content[e].b_name, 12));
            if (ename == name) {
                db.b_content[e].b_inodo = -1;
                memset(db.b_content[e].b_name, 0, 12);
                writeBlock(disk, sb, parent.i_block[b], &db);
                return true;
            }
        }
    }
    return false;
}

void MoveCmd::execute(const string& path, const string& destino) {

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

    // Verificar origen
    int srcInode = resolvePath(disk, sb, path);
    if (srcInode == -1) {
        cout << "ERROR: La ruta '" << path << "' no existe\n";
        fclose(disk); return;
    }

    // Verificar permiso escritura en origen
    Inode srcInodeData;
    readInode(disk, sb, srcInode, srcInodeData);
    if (!Permissions::canWrite(srcInodeData)) {
        cout << "ERROR: No tiene permiso de escritura sobre '" << path << "'\n";
        fclose(disk); return;
    }

    // Verificar destino
    int destInode = resolvePath(disk, sb, destino);
    if (destInode == -1) {
        cout << "ERROR: La carpeta destino '" << destino << "' no existe\n";
        fclose(disk); return;
    }

    // Verificar permiso escritura en destino
    Inode destInodeData;
    readInode(disk, sb, destInode, destInodeData);
    if (!Permissions::canWrite(destInodeData)) {
        cout << "ERROR: No tiene permiso de escritura en '" << destino << "'\n";
        fclose(disk); return;
    }

    // Obtener nombre y padre del origen
    string srcName = path;
    string srcParentPath = "/";
    size_t lastSlash = path.rfind('/');
    if (lastSlash != string::npos && lastSlash > 0) {
        srcParentPath = path.substr(0, lastSlash);
        srcName = path.substr(lastSlash + 1);
    } else if (lastSlash == 0) {
        srcName = path.substr(1);
    }

    int srcParentInode = resolvePath(disk, sb, srcParentPath);
    if (srcParentInode == -1) {
        cout << "ERROR: No se encontró carpeta padre\n";
        fclose(disk); return;
    }

    // Verificar que no exista el nombre en destino
    int existing = findInDirectory(disk, sb, destInode, srcName);
    if (existing != -1) {
        cout << "ERROR: Ya existe '" << srcName << "' en el destino\n";
        fclose(disk); return;
    }

    // MOVER = eliminar entrada del padre + agregar entrada en destino
    // Solo cambia referencias, no mueve bloques
    removeEntryByName(disk, sb, srcParentInode, srcName);
    addEntryToDirectory(disk, sb, part->start, destInode, srcName, srcInode);

    // Actualizar .. del inodo movido si es carpeta
    if (srcInodeData.i_type == '0' && srcInodeData.i_block[0] != -1) {
        DirectoryBlock db;
        readBlock(disk, sb, srcInodeData.i_block[0], &db);
        db.b_content[1].b_inodo = destInode; // actualizar ..
        writeBlock(disk, sb, srcInodeData.i_block[0], &db);
    }

    // Journaling
    JournalManager::write(disk, sb, part->start, "move", path, destino);

    writeSuperBlock(disk, part->start, sb);
    fclose(disk);

    cout << "OK: '" << path << "' movido a '" << destino << "'\n";
}