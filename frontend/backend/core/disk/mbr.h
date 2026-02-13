#ifndef MBR_H
#define MBR_H

#include <ctime>

struct Partition {
    char part_status = '0';
    char part_type = '0';
    char part_fit = '0';
    int part_start = -1;
    int part_size = 0;
    char part_name[16];
};

struct MBR {
    int mbr_tamano;
    time_t mbr_fecha_creacion;
    int mbr_dsk_signature;
    char dsk_fit;
    Partition mbr_partitions[4];
};

#endif
