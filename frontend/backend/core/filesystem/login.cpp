#include "login.h"
#include "../mount/mount_manager.h"
#include "../core/filesystem/session_manager.h"
#include "../core/filesystem/EXT2Utils.h"
#include "../core/filesystem/ext2_writer.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <cstdio>

using namespace std;

void Login::execute(const string& user, const string& pass, const string& id) {

    // Verificar que no haya una sesión activa ya
    if (SessionManager::isActive()) {
        cout << "ERROR: Ya hay una sesión activa. Ejecute 'logout' primero\n";
        return;
    }

    // Obtener la partición montada con ese ID
    MountedPartition* part = MountManager::getMountedById(id);
    if (!part) {
        cout << "ERROR: ID '" << id << "' no está montado\n";
        return;
    }

    FILE* disk = fopen(part->path.c_str(), "rb+");
    if (!disk) {
        cout << "ERROR: No se pudo abrir el disco\n";
        return;
    }

    SuperBlock sb;
    readSuperBlock(disk, part->start, sb);

    // Leer el archivo users.txt del filesystem
    string usersContent = EXT2Writer::readUsersFile(disk, sb, part->start);
    fclose(disk);

    if (usersContent.empty()) {
        cout << "ERROR: No se pudo leer users.txt\n";
        return;
    }

    // Parsear los datos del archivo users.txt
    // Formato: grupos como "id,G,nombre" y usuarios como "id,U,grupo,nombre,pass"
    struct UserEntry {
        int    uid;
        string group;
        string username;
        string password;
    };

    struct GroupEntry {
        int    gid;
        string name;
    };

    vector<UserEntry>  users;
    vector<GroupEntry> groups;

    istringstream stream(usersContent);
    string line;

    while (getline(stream, line)) {
        if (line.empty()) continue;

        // Parsear campos separados por coma
        vector<string> fields;
        stringstream ss(line);
        string field;
        while (getline(ss, field, ',')) {
            // Trim espacios
            while (!field.empty() && field.front() == ' ') field.erase(0,1);
            while (!field.empty() && field.back()  == ' ') field.pop_back();
            fields.push_back(field);
        }

        if (fields.size() < 3) continue;

        int  entryId = stoi(fields[0]);
        char type    = fields[1][0];

        if (type == 'G' && entryId > 0) {
            GroupEntry g;
            g.gid  = entryId;
            g.name = fields[2];
            groups.push_back(g);
        }
        else if (type == 'U' && entryId > 0 && fields.size() >= 5) {
            UserEntry u;
            u.uid      = entryId;
            u.group    = fields[2];
            u.username = fields[3];
            u.password = fields[4];
            users.push_back(u);
        }
    }

    // ===== BUSCAR USUARIO =====
    bool userFound = false;
    for (auto& u : users) {
        if (u.username == user) {
            userFound = true;

            // Validar contraseña (case sensitive)
            if (u.password != pass) {
                cout << "ERROR: Contraseña incorrecta\n";
                return;
            }

            // Buscar gid del grupo
            int gid = -1;
            for (auto& g : groups) {
                if (g.name == u.group) {
                    gid = g.gid;
                    break;
                }
            }

            // Iniciar sesión
            SessionManager::login(u.username, u.group, u.uid, gid, id);

            cout << "OK: Sesión iniciada\n";
            cout << "    Usuario : " << u.username << "\n";
            cout << "    Grupo   : " << u.group    << "\n";
            cout << "    ID      : " << id         << "\n";
            return;
        }
    }

    if (!userFound) {
        cout << "ERROR: El usuario '" << user << "' no existe\n";
    }
}