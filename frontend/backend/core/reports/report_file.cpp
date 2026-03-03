#include "report_file.h"
#include "report_utils.h"
#include "../core/filesystem/EXT2Utils.h"
#include "../core/filesystem/SuperBlock.h"
#include "../core/filesystem/Inode.h"
#include "../core/filesystem/Blocks.h"
#include <iostream>
#include <cstring>
using namespace std;

void ReportFile::generate(FILE* disk, MountedPartition* part,
                           const string& outPath, const string& filePath) {
    if (filePath.empty()) {
        cout << "ERROR: -path_file_ls es obligatorio para reporte file\n";
        return;
    }

    SuperBlock sb;
    readSuperBlock(disk, part->start, sb);

    int inodeNum = resolvePath(disk, sb, filePath);
    if (inodeNum == -1) {
        cout << "ERROR: El archivo '" << filePath << "' no existe\n";
        return;
    }

    Inode inode;
    readInode(disk, sb, inodeNum, inode);

    if (inode.i_type != '1') {
        cout << "ERROR: '" << filePath << "' no es un archivo\n";
        return;
    }

    // Leer contenido
    string content = "Archivo: " + filePath + "\n";
    content += string(40, '-') + "\n";

    string fileContent;
    for (int b = 0; b < 12; b++) {
        if (inode.i_block[b] == -1) break;
        FileBlock fb;
        memset(&fb, 0, sizeof(FileBlock));
        readBlock(disk, sb, inode.i_block[b], &fb);
        int toRead = min((int)sizeof(fb.b_content),
                         inode.i_size - (int)fileContent.size());
        if (toRead <= 0) break;
        fileContent.append(fb.b_content, toRead);
    }

    content += fileContent + "\n";
    writeTxt(content, outPath);
}