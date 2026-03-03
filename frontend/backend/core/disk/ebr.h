#ifndef EBR_H
#define EBR_H

#pragma pack(push, 1)

struct EBR {
    char      part_mount = '0';  // '0' = no montada, '1' = montada
    char      part_fit[3];       // "BF", "FF", "WF" — consistente con Partition
    long long part_start  = -1;
    long long part_size   = 0;
    long long part_next   = -1;  // -1 = no hay siguiente EBR
    char      part_name[16];
    char      part_correlative = '0';
    // Total: 1+3+8+8+8+16+1 = 45 bytes
};

#pragma pack(pop)
#endif