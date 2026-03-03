#ifndef REPORT_MBR_H
#define REPORT_MBR_H
#include <string>
#include <cstdio>
#include "../mount/mount_manager.h"
class ReportMbr {
public:
    static void generate(FILE* disk, MountedPartition* part,
                         const std::string& path);
};
#endif