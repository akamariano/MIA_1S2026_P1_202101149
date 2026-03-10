#ifndef REPORT_DISK_H
#define REPORT_DISK_H
#include <string>
#include <cstdio>
#include "../mount/mount_manager.h"
class ReportDisk {
public:
    static void generate(FILE* disk, MountedPartition* part,
                         const std::string& path);
};
#endif
