
#include "mkgrp.h"
#include "../filesystem/session_manager.h"
#include "../filesystem/users_manager.h"
#include "../filesystem/ext2_writer.h"
#include "../mount/mount_manager.h"
#include "../core/filesystem/EXT2Utils.h"
#include <iostream>
using namespace std;

void MkGrp::execute(const string& name) {

    if (!SessionManager::isActive()) {
        cout << "ERROR: No hay sesión activa\n"; return;
    }
    if (!SessionManager::isRoot()) {
        cout << "ERROR: Solo root puede ejecutar mkgrp\n"; return;
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

    // Verificar si ya existe (activo o inactivo)
    for (auto& g : data.groups) {
        if (g.name == name) {
            if (g.active) {
                cout << "ERROR: El grupo '" << name << "' ya existe\n";
                fclose(disk); return;
            } else {
                // Reutilizar grupo eliminado — reactivar
                g.gid    = nextGroupId(data);
                g.active = true;
                string newContent = serializeUsers(data);
                EXT2Writer::writeUsersFile(disk, sb, part->start, newContent);
                fclose(disk);
                cout << "OK: Grupo '" << name << "' creado correctamente\n";
                return;
            }
        }
    }

    // Nuevo grupo
    GroupEntry g;
    g.gid    = nextGroupId(data);
    g.name   = name;
    g.active = true;
    data.groups.push_back(g);

    string newContent = serializeUsers(data);
    EXT2Writer::writeUsersFile(disk, sb, part->start, newContent);
    fclose(disk);

    cout << "OK: Grupo '" << name << "' creado con ID " << g.gid << "\n";
}