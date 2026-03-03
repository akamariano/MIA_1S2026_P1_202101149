#ifndef REPORT_BM_H
#define REPORT_BM_H
#include <string>
#include <cstdio>
#include "../mount/mount_manager.h"

class ReportBm {
public:
    static void generateInodeBitmap(FILE* disk, MountedPartition* part,
                                    const std::string& path);
    static void generateBlockBitmap(FILE* disk, MountedPartition* part,
                                    const std::string& path);
};
#endif