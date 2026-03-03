#include "mkusr.h"
#include "../filesystem/session_manager.h"
#include "../filesystem/users_manager.h"
#include "../filesystem/ext2_writer.h"
#include "../mount/mount_manager.h"
#include "../core/filesystem/EXT2Utils.h"
#include <iostream>
using namespace std;

void MkUsr::execute(const string& user, const string& pass, const string& grp) {

    if (!SessionManager::isActive()) {
        cout << "ERROR: No hay sesión activa\n"; return;
    }
    if (!SessionManager::isRoot()) {
        cout << "ERROR: Solo root puede ejecutar mkusr\n"; return;
    }
    if (user.size() > 10) {
        cout << "ERROR: El nombre de usuario no puede exceder 10 caracteres\n"; return;
    }
    if (pass.size() > 10) {
        cout << "ERROR: La contraseña no puede exceder 10 caracteres\n"; return;
    }
    if (grp.size() > 10) {
        cout << "ERROR: El nombre de grupo no puede exceder 10 caracteres\n"; return;
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
        cout << "ERROR: El grupo '" << grp << "' no existe\n";
        fclose(disk); return;
    }

    // Verificar que el usuario no existe (activo o inactivo)
    for (auto& u : data.users) {
        if (u.username == user) {
            if (u.active) {
                cout << "ERROR: El usuario '" << user << "' ya existe\n";
            } else {
                // Reutilizar usuario eliminado
                u.uid      = nextUserId(data);
                u.group    = grp;
                u.password = pass;
                u.active   = true;
                string newContent = serializeUsers(data);
                EXT2Writer::writeUsersFile(disk, sb, part->start, newContent);
                cout << "OK: Usuario '" << user << "' creado correctamente\n";
            }
            fclose(disk); return;
        }
    }

    // Nuevo usuario
    UserEntry u;
    u.uid      = nextUserId(data);
    u.group    = grp;
    u.username = user;
    u.password = pass;
    u.active   = true;
    data.users.push_back(u);

    string newContent = serializeUsers(data);
    EXT2Writer::writeUsersFile(disk, sb, part->start, newContent);
    fclose(disk);

    cout << "OK: Usuario '" << user << "' creado con ID " << u.uid << "\n";
}