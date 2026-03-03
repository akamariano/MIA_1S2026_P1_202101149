#include "cat_cmd.h"
#include "../filesystem/session_manager.h"
#include "../mount/mount_manager.h"
#include "../filesystem/EXT2Utils.h"
#include "../filesystem/ext2_reader.h"
#include <iostream>
using namespace std;

void CatCmd::execute(const vector<string>& files) {

    if (!SessionManager::isActive()) {
        cout << "ERROR: No hay sesión activa\n"; return;
    }

    if (files.empty()) {
        cout << "ERROR: Debe especificar al menos -file1\n"; return;
    }

    MountedPartition* part = MountManager::getMountedById(
                                SessionManager::get().partitionId);
    if (!part) { cout << "ERROR: Partición no encontrada\n"; return; }

    FILE* disk = fopen(part->path.c_str(), "rb+");
    if (!disk) { cout << "ERROR: No se pudo abrir el disco\n"; return; }

    SuperBlock sb;
    readSuperBlock(disk, part->start, sb);

    // Leer y encadenar archivos
    bool first = true;
    for (auto& filepath : files) {
        string content = EXT2Reader::readFile(disk, sb, part->start, filepath);
        if (content.empty() && /* hubo error */ false) continue;

        if (!first) cout << "\n"; // separador entre archivos
        cout << content;
        first = false;
    }
    cout << "\n";

    fclose(disk);
}