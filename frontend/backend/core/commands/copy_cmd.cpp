#include "copy_cmd.h"
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
#include <string>
using namespace std;

// Copia recursiva de un inodo hacia un destino
static void copyRecursive(FILE* disk, SuperBlock& sb, long long partStart,
                          int srcInode, int destParentInode,
                          const string& name,
                          int uid, int gid) {
    Inode src;
    readInode(disk, sb, srcInode, src);

    // Verificar permiso de lectura en origen
    if (!Permissions::canRead(src)) {
        cout << "ADVERTENCIA: Sin permiso de lectura en '" << name << "', omitiendo\n";
        return;
    }

    if (src.i_type == '0') {
        // Es carpeta — crear en destino
        int newDir = EXT2Writer::createDirPublic(disk, sb, partStart,
                                                  destParentInode, name,
                                                  uid, gid);
        if (newDir == -1) return;

        // Copiar hijos recursivamente
        for (int b = 0; b < 12; b++) {
            if (src.i_block[b] == -1) break;
            DirectoryBlock db;
            memset(&db, 0, sizeof(DirectoryBlock));
            readBlock(disk, sb, src.i_block[b], &db);
            for (int e = 0; e < 4; e++) {
                if (db.b_content[e].b_inodo == -1) continue;
                string ename(db.b_content[e].b_name,
                             strnlen(db.b_content[e].b_name, 12));
                if (ename == "." || ename == "..") continue;
                copyRecursive(disk, sb, partStart,
                              db.b_content[e].b_inodo,
                              newDir, ename, uid, gid);
            }
        }
    } else {
        // Es archivo — leer contenido y crear copia
        string content;
        for (int b = 0; b < 12; b++) {
            if (src.i_block[b] == -1) break;
            FileBlock fb;
            memset(&fb, 0, sizeof(FileBlock));
            readBlock(disk, sb, src.i_block[b], &fb);
            int toRead = min((int)sizeof(fb.b_content),
                             src.i_size - (int)content.size());
            if (toRead <= 0) break;
            content.append(fb.b_content, toRead);
        }
        // Bloque indirecto
        if (src.i_block[12] != -1) {
            PointerBlock pb;
            readBlock(disk, sb, src.i_block[12], &pb);
            for (int i = 0; i < 16; i++) {
                if (pb.b_pointers[i] == -1) break;
                FileBlock fb;
                memset(&fb, 0, sizeof(FileBlock));
                readBlock(disk, sb, pb.b_pointers[i], &fb);
                int toRead = min((int)sizeof(fb.b_content),
                                 src.i_size - (int)content.size());
                if (toRead <= 0) break;
                content.append(fb.b_content, toRead);
            }
        }

        // Construir path destino y crear archivo
        EXT2Writer::mkfile(disk, sb, partStart,
                           name, false, 0, content, uid, gid);
    }
}

void CopyCmd::execute(const string& path, const string& destino) {

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
        cout << "ERROR: La ruta origen '" << path << "' no existe\n";
        fclose(disk); return;
    }

    // Verificar destino existe
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

    // Obtener nombre del origen
    string srcName = path;
    size_t lastSlash = path.rfind('/');
    if (lastSlash != string::npos)
        srcName = path.substr(lastSlash + 1);

    int uid = SessionManager::get().uid;
    int gid = SessionManager::get().gid;

    // Copiar recursivamente
    copyRecursive(disk, sb, part->start,
                  srcInode, destInode, srcName, uid, gid);

    // Journaling
    JournalManager::write(disk, sb, part->start, "copy", path, destino);

    writeSuperBlock(disk, part->start, sb);
    fclose(disk);

    cout << "OK: '" << path << "' copiado a '" << destino << "'\n";
}