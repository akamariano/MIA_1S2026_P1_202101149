#ifndef REPORT_FILE_H
#define REPORT_FILE_H
#include <string>
#include <cstdio>
#include "../mount/mount_manager.h"
class ReportFile {
public:
    static void generate(FILE* disk, MountedPartition* part,
                         const std::string& outPath,
                         const std::string& filePath);
};
#endif