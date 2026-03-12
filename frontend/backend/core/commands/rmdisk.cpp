#include "commands/rmdisk.h"
#include "../core/mount/mount_manager.h"
#include <iostream>
#include <cstdio>
#include <fstream>

void RmDisk::execute(std::string path) {
    // Verificar si existe antes de intentar borrar
    if (std::remove(path.c_str()) == 0) {
        MountManager::unmountByPath(path);
        std::cout << "OK: Disco eliminado correctamente\n";
    } else {
        std::cout << "ERROR: El disco no existe en la ruta especificada\n";
    }
}