#include "mkfile_cmd.h"
#include "../filesystem/session_manager.h"
#include "../filesystem/ext2_writer.h"
#include "../mount/mount_manager.h"
#include "../filesystem/EXT2Utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

void MkfileCmd::execute(const string& path, bool createParents,
                         int size, const string& cont) {

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

    // ===== DETERMINAR CONTENIDO =====
    string content = "";

    if (!cont.empty()) {
        // -cont tiene prioridad sobre -size
        ifstream contFile(cont);
        if (!contFile.is_open()) {
            cout << "ERROR: No se pudo abrir el archivo: " << cont << "\n";
            fclose(disk); return;
        }
        ostringstream ss;
        ss << contFile.rdbuf();
        content = ss.str();
    } else if (size > 0) {
        // Generar contenido 0123456789 repetido
        for (int i = 0; i < size; i++)
            content += (char)('0' + (i % 10));
    }

    EXT2Writer::mkfile(disk, sb, part->start, path, createParents,
                       size, content,
                       SessionManager::get().uid,
                       SessionManager::get().gid);

    fclose(disk);
}