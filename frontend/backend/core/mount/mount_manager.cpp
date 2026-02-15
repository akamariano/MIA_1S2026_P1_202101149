#include "mount_manager.h"
#include <iostream>
#include "disk/mbr.h"

#include <cstring>

std::vector<MountedPartition> MountManager::mountedPartitions;

// ================== MOUNT ==================
std::string MountManager::mount(std::string path, std::string name) {

    // 🔒 VALIDAR DOBLE MOUNT
    for (auto &m : mountedPartitions) {
        if (m.path == path && m.name == name) {
            std::cout << "ERROR: La partición ya está montada\n";
            return "";
        }
    }

    FILE* file = fopen(path.c_str(), "rb");
    if (!file) {
        std::cout << "ERROR: No se pudo abrir el disco\n";
        return "";
    }

    MBR mbr;
    fread(&mbr, sizeof(MBR), 1, file);

    int partStart = -1;
    int partSize = -1;

    for (int i = 0; i < 4; i++) {
        if (mbr.mbr_partitions[i].part_status == '1' &&
            strncmp(mbr.mbr_partitions[i].part_name,
                    name.c_str(), 16) == 0) {

            partStart = mbr.mbr_partitions[i].part_start;
            partSize  = mbr.mbr_partitions[i].part_size;
            break;
        }
    }

    fclose(file);

    if (partStart == -1) {
        std::cout << "ERROR: Partición no encontrada\n";
        return "";
    }

    // ===== GENERAR ID =====

    std::string carnet = "49";
    char letter = 'A';
    int number = 1;

    bool diskExists = false;

    for (auto &m : mountedPartitions) {
        if (m.path == path) {
            diskExists = true;
            letter = m.id.back();
            break;
        }
    }

    if (diskExists) {
        int count = 0;
        for (auto &m : mountedPartitions) {
            if (m.path == path) count++;
        }
        number = count + 1;
    } else {
        char maxLetter = 'A' - 1;
        for (auto &m : mountedPartitions) {
            char currentLetter = m.id.back();
            if (currentLetter > maxLetter)
                maxLetter = currentLetter;
        }

        if (maxLetter >= 'A')
            letter = maxLetter + 1;
        else
            letter = 'A';

        number = 1;
    }

    std::string newId = carnet + std::to_string(number) + letter;

    // ===== GUARDAR TODO =====

    MountedPartition mp;
    mp.path = path;
    mp.name = name;
    mp.id = newId;
    mp.start = partStart;
    mp.size = partSize;

    mountedPartitions.push_back(mp);

    return newId;
}

MountedPartition* MountManager::getMountedById(std::string id) {

    for (auto &m : mountedPartitions) {
        if (m.id == id) {
            return &m;
        }
    }

    return nullptr;
}

// ================== SHOW MOUNTED ==================
void MountManager::showMounted() {

    std::cout << "\n--- PARTICIONES MONTADAS ---\n";

    if (mountedPartitions.empty()) {
        std::cout << "No hay particiones montadas\n";
        return;
    }

    for (auto &m : mountedPartitions) {
        std::cout << "ID: " << m.id
                  << " | Path: " << m.path
                  << " | Name: " << m.name << "\n";
    }
}
