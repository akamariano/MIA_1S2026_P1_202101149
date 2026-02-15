#ifndef MBR_H
#define MBR_H

#include <ctime>

#pragma pack(push,1)// evita padding automático del compilador
//STRUCTS DE PARTICIONES Y MBR
struct Partition {
    char part_status = '0';
    char part_type = '0';
    char part_fit[3];  // "BF", "FF", "WF" tiene 3 en caso de 1 caracter null pero puede modificarse
    int part_start = -1;
    int part_size = 0;
    char part_name[16];
    //total 9 bytes
};

struct MBR {
    int mbr_tamano;//4byte
    time_t mbr_fecha_creacion;//8byte
    int mbr_dsk_signature;//4byte
    char dsk_fit[3];   //  NO char simple 3byte
    Partition mbr_partitions[4];//4byte *29, 116byte
    //135 bytes
};

#pragma pack(pop)

#endif
