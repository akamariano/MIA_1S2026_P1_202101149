#include "mount_manager.h"
#include <iostream>
#include <algorithm>
#include "../disk/mbr.h"
#include <cstring>
#include <map>

std::vector<MountedPartition> MountManager::mountedPartitions;

std::string MountManager::mount(std::string path, std::string name) {

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

    long long partStart = -1;
    long long partSize  = -1;

    for (int i = 0; i < 4; i++) {
        if (mbr.mbr_partitions[i].part_status == '1' &&
            strncmp(mbr.mbr_partitions[i].part_name, name.c_str(), 16) == 0) {
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

    std::map<std::string, char> diskLetter;
    char nextLetter = 'A';

    for (auto &m : mountedPartitions) {
        if (diskLetter.find(m.path) == diskLetter.end()) {
            diskLetter[m.path] = m.id.back();
        }
        if (m.id.back() >= nextLetter)
            nextLetter = m.id.back() + 1;
    }

    char assignedLetter;
    int  assignedNumber;

    if (diskLetter.find(path) != diskLetter.end()) {
        assignedLetter = diskLetter[path];
        int count = 0;
        for (auto &m : mountedPartitions)
            if (m.path == path) count++;
        assignedNumber = count + 1;
    } else {
        assignedLetter = nextLetter;
        assignedNumber = 1;
    }

    std::string newId = carnet + std::to_string(assignedNumber) + assignedLetter;

    MountedPartition mp;
    mp.path  = path;
    mp.name  = name;
    mp.id    = newId;
    mp.start = partStart;
    mp.size  = partSize;
    mountedPartitions.push_back(mp);

    std::cout << "OK: Partición montada con ID: " << newId << "\n";
    return newId;
}

MountedPartition* MountManager::getMountedById(std::string id) {
    for (auto &m : mountedPartitions) {
        if (m.id == id) return &m;
    }
    return nullptr;
}

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

void MountManager::unmountByPath(std::string path) {
    mountedPartitions.erase(
        std::remove_if(mountedPartitions.begin(), mountedPartitions.end(),
            [&path](const MountedPartition& m) { return m.path == path; }),
        mountedPartitions.end()
    );
}