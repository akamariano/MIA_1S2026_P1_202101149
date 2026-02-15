#ifndef FDISK_H
#define FDISK_H

#include <string>

class FDisk {
public:
    bool existeNombreLogica(FILE *disk, int startExtendida, std::string nombre);
    void printLogicas(FILE *disk, int startExtendida);
    void execute(int size, char unit, std::string path,
                 char type, std::string fit, std::string name);
};

#endif
