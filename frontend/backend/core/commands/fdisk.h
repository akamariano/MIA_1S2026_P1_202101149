#ifndef FDISK_H
#define FDISK_H
#include <string>
#include <cstdio>

class FDisk {
public:
    bool existeNombreLogica(FILE* disk, long long startExtendida, std::string nombre);
    void printLogicas(FILE* disk, long long startExtendida);
    void execute(int size, char unit, std::string path,
                 char type, std::string fit, std::string name);
};
#endif