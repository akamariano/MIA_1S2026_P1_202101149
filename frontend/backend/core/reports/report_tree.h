#ifndef REPORT_TREE_H
#define REPORT_TREE_H
#include <string>
#include <cstdio>
#include "../mount/mount_manager.h"
class ReportTree {
public:
    static void generate(FILE* disk, MountedPartition* part,
                         const std::string& path);
};
#endif
