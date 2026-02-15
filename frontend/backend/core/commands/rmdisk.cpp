#include "commands/rmdisk.h"
#include <iostream>
#include <cstdio>
#include <fstream>

void RmDisk::execute(std::string path) {

    // ===== Verificar existencia =====
    std::ifstream file(path);

    if (!file.is_open()) {
        std::cout << "ERROR: El disco no existe en la ruta especificada\n";
        return;
    }

    file.close();

    // ===== Confirmación =====
    std::cout << "¿Está seguro que desea eliminar el disco? (Y/N): ";
    char response;
    std::cin >> response;
    std::cin.ignore();

    if (toupper(response) != 'Y') {
        std::cout << "Operación cancelada\n";
        return;
    }

    // ===== Eliminar archivo =====
    if (std::remove(path.c_str()) == 0) {
        std::cout << "Disco eliminado correctamente\n";
    } else {
        std::cout << "ERROR: No se pudo eliminar el disco\n";
    }
}
