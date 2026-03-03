#include "report_bm.h"
#include "report_utils.h"
#include "../core/filesystem/EXT2Utils.h"
#include "../core/filesystem/SuperBlock.h"
#include <string>
#include <cstring>
using namespace std;

void ReportBm::generateInodeBitmap(FILE* disk, MountedPartition* part,
                                    const string& path) {
    SuperBlock sb;
    readSuperBlock(disk, part->start, sb);

    string content = "BITMAP DE INODOS\n";
    content += "Total inodos: " + to_string(sb.s_inodes_count) + "\n\n";

    for (int i = 0; i < sb.s_inodes_count; i++) {
        fseek(disk, sb.s_bm_inode_start + i, SEEK_SET);
        unsigned char val;
        fread(&val, 1, 1, disk);
        content += (val ? "1" : "0");
        if ((i + 1) % 20 == 0) content += "\n";
        else content += " ";
    }
    content += "\n";

    writeTxt(content, path);
}

void ReportBm::generateBlockBitmap(FILE* disk, MountedPartition* part,
                                    const string& path) {
    SuperBlock sb;
    readSuperBlock(disk, part->start, sb);

    string content = "BITMAP DE BLOQUES\n";
    content += "Total bloques: " + to_string(sb.s_blocks_count) + "\n\n";

    for (int i = 0; i < sb.s_blocks_count; i++) {
        fseek(disk, sb.s_bm_block_start + i, SEEK_SET);
        unsigned char val;
        fread(&val, 1, 1, disk);
        content += (val ? "1" : "0");
        if ((i + 1) % 20 == 0) content += "\n";
        else content += " ";
    }
    content += "\n";

    writeTxt(content, path);
}