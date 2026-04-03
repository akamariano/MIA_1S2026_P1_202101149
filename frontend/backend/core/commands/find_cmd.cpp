#include "find_cmd.h"
#include "../filesystem/session_manager.h"
#include "../filesystem/permissions.h"
#include "../filesystem/EXT2Utils.h"
#include "../mount/mount_manager.h"
#include "../core/filesystem/SuperBlock.h"
#include "../core/filesystem/Inode.h"
#include "../core/filesystem/Blocks.h"
#include <iostream>
#include <cstring>
#include <string>
#include <vector>
using namespace std;

// Verifica si un nombre coincide con un patrón que soporta ? y *
static bool matchPattern(const string& pattern, const string& name) {
    int p = 0, n = 0;
    int starP = -1, starN = -1;

    while (n < (int)name.size()) {
        if (p < (int)pattern.size() &&
            (pattern[p] == '?' || pattern[p] == name[n])) {
            p++; n++;
        } else if (p < (int)pattern.size() && pattern[p] == '*') {
            starP = p++;
            starN = n;
        } else if (starP != -1) {
            p = starP + 1;
            n = ++starN;
        } else {
            return false;
        }
    }
    while (p < (int)pattern.size() && pattern[p] == '*') p++;
    return p == (int)pattern.size();
}

// Búsqueda recursiva
static void findRecursive(FILE* disk, SuperBlock& sb,
                          int inodeNum, const string& currentPath,
                          const string& pattern,
                          vector<string>& results) {
    Inode inode;
    readInode(disk, sb, inodeNum, inode);

    if (inode.i_type != '0') return;

    for (int b = 0; b < 12; b++) {
        if (inode.i_block[b] == -1) break;
        DirectoryBlock db;
        memset(&db, 0, sizeof(DirectoryBlock));
        readBlock(disk, sb, inode.i_block[b], &db);

        for (int e = 0; e < 4; e++) {
            if (db.b_content[e].b_inodo == -1) continue;
            string ename(db.b_content[e].b_name,
                         strnlen(db.b_content[e].b_name, 12));
            if (ename == "." || ename == "..") continue;

            string fullPath = (currentPath == "/")
                            ? "/" + ename
                            : currentPath + "/" + ename;

            Inode child;
            readInode(disk, sb, db.b_content[e].b_inodo, child);

            // Verificar permiso de lectura
            if (!Permissions::canRead(child)) continue;

            // Si coincide con el patrón, agregar a resultados
            if (matchPattern(pattern, ename)) {
                results.push_back(fullPath);
            }

            // Si es carpeta, buscar recursivamente
            if (child.i_type == '0') {
                findRecursive(disk, sb, db.b_content[e].b_inodo,
                              fullPath, pattern, results);
            }
        }
    }
}

void FindCmd::execute(const string& path, const string& name) {

    if (!SessionManager::isActive()) {
        cout << "ERROR: No hay sesión activa\n"; return;
    }

    MountedPartition* part = MountManager::getMountedById(
                                SessionManager::get().partitionId);
    if (!part) { cout << "ERROR: Partición no encontrada\n"; return; }

    FILE* disk = fopen(part->path.c_str(), "rb");
    if (!disk) { cout << "ERROR: No se pudo abrir el disco\n"; return; }

    SuperBlock sb;
    readSuperBlock(disk, part->start, sb);

    // Resolver inodo del path de búsqueda
    int startInode = resolvePath(disk, sb, path);
    if (startInode == -1) {
        cout << "ERROR: La ruta '" << path << "' no existe\n";
        fclose(disk); return;
    }

    Inode startInodeData;
    readInode(disk, sb, startInode, startInodeData);
    if (startInodeData.i_type != '0') {
        cout << "ERROR: '" << path << "' no es una carpeta\n";
        fclose(disk); return;
    }

    // Buscar recursivamente
    vector<string> results;
    findRecursive(disk, sb, startInode, path, name, results);

    fclose(disk);

    if (results.empty()) {
        cout << "No se encontraron resultados para '" << name << "'\n";
        return;
    }

    cout << "Resultados de búsqueda '" << name << "' en '" << path << "':\n";
    for (const string& r : results) {
        cout << "  " << r << "\n";
    }
    cout << "Total: " << results.size() << " resultado(s)\n";
}