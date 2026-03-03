// chgrp.cpp
#include "chgrp.h"
#include "../filesystem/session_manager.h"
#include "../filesystem/users_manager.h"
#include "../filesystem/ext2_writer.h"
#include "../mount/mount_manager.h"
#include "../core/filesystem/EXT2Utils.h"
#include <iostream>
using namespace std;

void ChGrp::execute(const string& user, const string& grp) {

    if (!SessionManager::isActive()) {
        cout << "ERROR: No hay sesión activa\n"; return;
    }
    if (!SessionManager::isRoot()) {
        cout << "ERROR: Solo root puede ejecutar chgrp\n"; return;
    }

    MountedPartition* part = MountManager::getMountedById(
                                SessionManager::get().partitionId);
    if (!part) { cout << "ERROR: Partición no encontrada\n"; return; }

    FILE* disk = fopen(part->path.c_str(), "rb+");
    if (!disk) { cout << "ERROR: No se pudo abrir el disco\n"; return; }

    SuperBlock sb;
    readSuperBlock(disk, part->start, sb);

    string content = EXT2Writer::readUsersFile(disk, sb, part->start);
    UsersData data = parseUsers(content);

    // Verificar que el grupo existe y está activo
    bool groupExists = false;
    for (auto& g : data.groups)
        if (g.name == grp && g.active) { groupExists = true; break; }

    if (!groupExists) {
        cout << "ERROR: El grupo '" << grp << "' no existe o fue eliminado\n";
        fclose(disk); return;
    }

    // Buscar usuario
    bool found = false;
    for (auto& u : data.users) {
        if (u.username == user && u.active) {
            u.group = grp;
            found = true;
            break;
        }
    }

    if (!found) {
        cout << "ERROR: El usuario '" << user << "' no existe\n";
        fclose(disk); return;
    }

    string newContent = serializeUsers(data);
    EXT2Writer::writeUsersFile(disk, sb, part->start, newContent);
    fclose(disk);

    cout << "OK: Usuario '" << user << "' movido al grupo '" << grp << "'\n";
}