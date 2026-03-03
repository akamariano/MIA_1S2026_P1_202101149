#include "rmusr.h"
#include "../filesystem/session_manager.h"
#include "../filesystem/users_manager.h"
#include "../filesystem/ext2_writer.h"
#include "../mount/mount_manager.h"
#include "../core/filesystem/EXT2Utils.h"
#include <iostream>
using namespace std;

void RmUsr::execute(const string& user) {

    if (!SessionManager::isActive()) {
        cout << "ERROR: No hay sesión activa\n"; return;
    }
    if (!SessionManager::isRoot()) {
        cout << "ERROR: Solo root puede ejecutar rmusr\n"; return;
    }
    if (user == "root") {
        cout << "ERROR: No se puede eliminar el usuario root\n"; return;
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

    bool found = false;
    for (auto& u : data.users) {
        if (u.username == user && u.active) {
            u.uid    = 0;
            u.active = false;
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

    cout << "OK: Usuario '" << user << "' eliminado correctamente\n";
}