#include "mkdir_cmd.h"
#include "../filesystem/session_manager.h"
#include "../filesystem/ext2_writer.h"
#include "../mount/mount_manager.h"
#include "../filesystem/EXT2Utils.h"
#include <iostream>
using namespace std;

void MkdirCmd::execute(const string& path, bool createParents) {

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

    EXT2Writer::mkdir(disk, sb, part->start, path, createParents,
                      SessionManager::get().uid,
                      SessionManager::get().gid);

    fclose(disk);
}