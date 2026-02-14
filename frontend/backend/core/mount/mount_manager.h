#ifndef MOUNT_MANAGER_H
#define MOUNT_MANAGER_H

#include <string>
#include <vector>

struct MountedPartition {
    std::string path;
    std::string name;
    std::string id;
};

class MountManager {
public:
    static std::vector<MountedPartition> mountedPartitions;

    static std::string mount(std::string path, std::string name);
    static void showMounted();
};

#endif
