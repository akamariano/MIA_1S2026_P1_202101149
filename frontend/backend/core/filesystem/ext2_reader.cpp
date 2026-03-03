#include "ext2_reader.h"
#include "EXT2Utils.h"
#include "../filesystem/session_manager.h"
#include "../filesystem/permissions.h"
#include <iostream>
#include <cstring>
using namespace std;

string EXT2Reader::readFile(FILE* disk, SuperBlock& sb,
                             long long partStart,
                             const string& path) {
    // Resolver ruta
    int inodeNum = resolvePath(disk, sb, path);
    if (inodeNum == -1) {
        cout << "ERROR: El archivo '" << path << "' no existe\n";
        return "";
    }

    Inode inode;
    readInode(disk, sb, inodeNum, inode);

    // Verificar que es archivo
    if (inode.i_type != '1') {
        cout << "ERROR: '" << path << "' no es un archivo\n";
        return "";
    }

    // Verificar permiso de lectura
    if (!Permissions::canRead(inode)) {
        cout << "ERROR: Sin permiso de lectura en '" << path << "'\n";
        return "";
    }

    // Leer contenido de bloques directos
    string result;
    for (int b = 0; b < 12; b++) {
        if (inode.i_block[b] == -1) break;

        FileBlock fb;
        memset(&fb, 0, sizeof(FileBlock));
        readBlock(disk, sb, inode.i_block[b], &fb);

        int toRead = min((int)sizeof(fb.b_content),
                         inode.i_size - (int)result.size());
        if (toRead <= 0) break;
        result.append(fb.b_content, toRead);
    }

    // Actualizar atime
    inode.i_atime = time(nullptr);
    writeInode(disk, sb, inodeNum, inode);

    return result;
}