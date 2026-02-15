#include "mount_manager.h"
#include <iostream>

std::vector<MountedPartition> MountManager::mountedPartitions;

// ================== MOUNT ==================
std::string MountManager::mount(std::string path, std::string name) {

    // 🔒 VALIDAR DOBLE MOUNT
    for (auto &m : mountedPartitions) {
        if (m.path == path && m.name == name) {
            std::cout << "ERROR: La partición ya está montada\n";
            return "";   // ← si ya existe, no seguimos
        }
    }

    std::string carnet = "49";

    char letter = 'A';
    int number = 1;

    bool diskExists = false;

    // 🔎 Verificar si el disco ya tiene letra asignada
    for (auto &m : mountedPartitions) {
        if (m.path == path) {
            diskExists = true;
            letter = m.id.back();  // misma letra del disco
            break;
        }
    }

    if (diskExists) {

        // 📌 Contar particiones ya montadas del mismo disco
        int count = 0;
        for (auto &m : mountedPartitions) {
            if (m.path == path) {
                count++;
            }
        }

        number = count + 1;

    } else {

        // 📌 Buscar la letra más alta usada
        char maxLetter = 'A' - 1;

        for (auto &m : mountedPartitions) {
            char currentLetter = m.id.back();
            if (currentLetter > maxLetter) {
                maxLetter = currentLetter;
            }
        }

        if (maxLetter >= 'A')
            letter = maxLetter + 1;
        else
            letter = 'A';

        number = 1;
    }

    std::string newId = carnet + std::to_string(number) + letter;

    MountedPartition mp;
    mp.path = path;
    mp.name = name;
    mp.id = newId;

    mountedPartitions.push_back(mp);

    return newId;
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
