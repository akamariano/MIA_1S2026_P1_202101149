#ifndef FDISK_H
#define FDISK_H

#include <string>

class FDisk {
public:
    void execute(int size, char unit, std::string path,
                 char type, std::string fit, std::string name);
};

#endif
