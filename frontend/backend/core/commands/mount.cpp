#include "mount.h"
#include "../mount/mount_manager.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include "../disk/mbr.h"

void Mount::execute(std::string path, std::string name) {

    FILE* file = fopen(path.c_str(), "rb");

    if (!file) {
        std::cout << "ERROR: El disco no existe\n";
        return;
    }

    MBR mbr;
    fread(&mbr, sizeof(MBR), 1, file);

    bool found = false;

    for (int i = 0; i < 4; i++) {
        if (mbr.mbr_partitions[i].part_status == '1' &&
            mbr.mbr_partitions[i].part_type == 'P') {

            if (strncmp(mbr.mbr_partitions[i].part_name,
                        name.c_str(), 16) == 0) {
                found = true;
                break;
            }
        }
    }

    fclose(file);

    if (!found) {
        std::cout << "ERROR: Partición no encontrada\n";
        return;
    }

    std::string id = MountManager::mount(path, name);

    std::cout << "Partición montada correctamente\n";
    std::cout << "ID asignado: " << id << "\n";

    MountManager::showMounted();
}
