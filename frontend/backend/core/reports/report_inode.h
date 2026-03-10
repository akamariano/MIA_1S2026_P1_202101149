#ifndef REPORT_INODE_H
#define REPORT_INODE_H
#include <string>
#include <cstdio>
#include "../mount/mount_manager.h"
class ReportInode {
public:
    static void generate(FILE* disk, MountedPartition* part,
                         const std::string& path);
};
#endif
