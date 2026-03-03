#ifndef REPORT_SB_H
#define REPORT_SB_H
#include <string>
#include <cstdio>
#include "../mount/mount_manager.h"
class ReportSb {
public:
    static void generate(FILE* disk, MountedPartition* part,
                         const std::string& path);
};
#endif