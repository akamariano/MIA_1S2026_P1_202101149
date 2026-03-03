#ifndef MBR_H
#define MBR_H
#include <ctime>

#pragma pack(push, 1)

struct Partition {
    char part_status = '0';   // '0' = libre, '1' = ocupada
    char part_type   = '0';   // 'P' = primaria, 'E' = extendida, 'L' = lógica
    char part_fit[3];         // "BF", "FF", "WF" + '\0'
    long long part_start = -1;
    long long part_size  = 0;
    char part_name[16];
    char part_correlative = '0'; // correlativo para mount
    // Total: 1+1+3+8+8+16+1 = 38 bytes
};

struct MBR {
    long long mbr_tamano;          // 8 bytes — soporta discos grandes
    time_t    mbr_fecha_creacion;  // 8 bytes
    int       mbr_dsk_signature;   // 4 bytes
    char      dsk_fit[3];          // 3 bytes
    Partition mbr_partitions[4];   // 38 * 4 = 152 bytes
    // Total: 8+8+4+3+152 = 175 bytes
};

#pragma pack(pop)
#endif