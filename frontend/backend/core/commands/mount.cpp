#include "mount.h"
#include "../mount/mount_manager.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cctype>         
#include "../disk/mbr.h"

/**
 * Función para limpiar el path.
 * Normaliza separadores y elimina caracteres innecesarios.
 */
std::string cleanPath(std::string path) {

    while (!path.empty() && isspace(path.front()))
        path.erase(path.begin());

    while (!path.empty() && isspace(path.back()))
        path.pop_back();

    if (!path.empty() && path.front() == '"')
        path.erase(0, 1);

    if (!path.empty() && path.back() == '"')
        path.pop_back();

    return path;
}
void Mount::execute(std::string path, std::string name) {

    path = cleanPath(path);

    FILE* file = fopen(path.c_str(), "rb");
    if (!file) {
        std::cout << "ERROR: El disco no existe\n";
        std::cout << "Path recibido: [" << path << "]\n";
        return;
    }

    MBR mbr;
    fread(&mbr, sizeof(MBR), 1, file);
    fclose(file);

    bool found = false;

    // Solo particiones PRIMARIAS pueden montarse
    for (int i = 0; i < 4; i++) {
        if (mbr.mbr_partitions[i].part_status == '1' &&
            mbr.mbr_partitions[i].part_type == 'P') {
            if (strncmp(mbr.mbr_partitions[i].part_name, name.c_str(), 16) == 0) {
                found = true;
                break;
            }
        }
    }

    if (!found) {
        std::cout << "ERROR: Partición primaria '" << name << "' no encontrada\n";
        return;
    }

    std::string id = MountManager::mount(path, name);
    if (id.empty()) return;

    std::cout << "OK: Partición montada correctamente\n";
    std::cout << "ID asignado: " << id << "\n";
    MountManager::showMounted();
}
