#ifndef REPORT_LS_H
#define REPORT_LS_H
#include <string>
#include <cstdio>
#include "../mount/mount_manager.h"
class ReportLs {
public:
    static void generate(FILE* disk, MountedPartition* part,
                         const std::string& outPath,
                         const std::string& dirPath);
};
#endif
