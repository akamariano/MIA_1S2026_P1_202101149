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
    // Buscar el inode del archivo usando la ruta
    int inodeNum = resolvePath(disk, sb, path);
    if (inodeNum == -1) {
        cout << "ERROR: El archivo '" << path << "' no existe\n";
        return "";
    }

    Inode inode;
    readInode(disk, sb, inodeNum, inode);

    // Validar que sea un archivo regular (no un directorio)
    if (inode.i_type != '1') {
        cout << "ERROR: '" << path << "' no es un archivo\n";
        return "";
    }

    // Verificar que el usuario actual tiene permiso de lectura
    if (!Permissions::canRead(inode)) {
        cout << "ERROR: Sin permiso de lectura en '" << path << "'\n";
        return "";
    }

    // Leer contenido desde los bloques directos del inode
    string result;
    for (int b = 0; b < 12; b++) {
        if (inode.i_block[b] == -1) break;

        FileBlock fb;
        memset(&fb, 0, sizeof(FileBlock));
        readBlock(disk, sb, inode.i_block[b], &fb);

        // Leer solo hasta el tamaño del archivo
        int toRead = min((int)sizeof(fb.b_content),
                         inode.i_size - (int)result.size());
        if (toRead <= 0) break;
        result.append(fb.b_content, toRead);
    }

    // Actualizar tiempo de último acceso
    inode.i_atime = time(nullptr);
    writeInode(disk, sb, inodeNum, inode);

    return result;
}