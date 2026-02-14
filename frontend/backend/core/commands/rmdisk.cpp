#include "commands/rmdisk.h"
#include <iostream>
#include <cstdio>
#include <fstream>

void RmDisk::execute(std::string path) {

    std::ifstream file(path);

    if (!file.good()) {
        std::cout << "Error: el disco no existe\n";
        return;
    }

    file.close();

    if (std::remove(path.c_str()) == 0) {
        std::cout << "Disco eliminado correctamente\n";
    } else {
        std::cout << "Error al eliminar el disco\n";
    }
}
