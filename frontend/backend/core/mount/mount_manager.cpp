#include "mount_manager.h"
#include <iostream>

std::vector<MountedPartition> MountManager::mountedPartitions;

// ================== MOUNT ==================
std::string MountManager::mount(std::string path, std::string name) {

    std::string carnet = "49";  // últimos 2 dígitos
    char letter = 'A';
    int number = 1;

    // 🔎 Buscar si el disco ya fue montado antes
    bool diskExists = false;
    char diskLetter = 'A';

    for (auto &m : mountedPartitions) {
        if (m.path == path) {
            diskExists = true;
            diskLetter = m.id.back();
            break;
        }
    }

    if (diskExists) {
        letter = diskLetter;

        // contar cuántas particiones de ese disco ya están montadas
        for (auto &m : mountedPartitions) {
            if (m.path == path) {
                number++;
            }
        }
    } else {
        // nuevo disco → buscar siguiente letra disponible
        if (!mountedPartitions.empty()) {
            char lastLetter = mountedPartitions.back().id.back();
            letter = lastLetter + 1;
        }
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
