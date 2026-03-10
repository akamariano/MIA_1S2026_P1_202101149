#ifndef REPORT_BLOCK_H
#define REPORT_BLOCK_H
#include <string>
#include <cstdio>
#include "../mount/mount_manager.h"
class ReportBlock {
public:
    static void generate(FILE* disk, MountedPartition* part,
                         const std::string& path);
};
#endif
