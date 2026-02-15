#ifndef MOUNT_MANAGER_H
#define MOUNT_MANAGER_H

#include <vector>
#include <string>

struct MountedPartition {
    std::string path;
    std::string name;
    std::string id;
    int start;
    int size;
};

class MountManager {
private:
    static std::vector<MountedPartition> mountedPartitions;

public:
    static std::string mount(std::string path, std::string name);
    static void showMounted();

    // 🔥 AGREGAR ESTA LÍNEA
    static MountedPartition* getMountedById(std::string id);
};

#endif
