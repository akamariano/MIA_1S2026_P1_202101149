#ifndef INODE_H
#define INODE_H

#include <ctime>

#pragma pack(push, 1)

struct Inode {

    int i_uid;              // ID del usuario propietario
    int i_gid;              // ID del grupo propietario
    int i_size;             // Tamaño del archivo en bytes

    time_t i_atime;         // Última lectura
    time_t i_ctime;         // Fecha de creación
    time_t i_mtime;         // Última modificación

    int i_block[15];        // 12 directos + 3 indirectos

    char i_type;            // '0' = carpeta, '1' = archivo
    int i_perm;             // Permisos (ej: 664)

};

#pragma pack(pop)

#endif
