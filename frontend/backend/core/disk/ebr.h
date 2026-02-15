#ifndef EBR_H
#define EBR_H

#pragma pack(push,1)

struct EBR {
    char part_mount;     // '0' o '1' valor booleano básicamente
    char part_fit;       // 'B', 'F', 'W'
    int part_start;      
    int part_s;          
    int part_next;       
    char part_name[16];
};

#pragma pack(pop)

#endif
