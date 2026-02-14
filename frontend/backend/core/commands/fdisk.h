#ifndef FDISK_H
#define FDISK_H

#include <string>

class FDisk {
public:
    void execute(int size, char unit, std::string path,
                 char type, char fit, std::string name);
};

#endif
