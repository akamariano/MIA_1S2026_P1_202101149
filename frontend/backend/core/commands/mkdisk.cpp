#include "commands/mkdisk.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include "disk/mbr.h"
#include <ctime>
#include <cstdlib>

void MkDisk::execute(int size, char unit, char fit, std::string path) {

    int bytes = 0;

    if (unit == 'K' || unit == 'k') {
        bytes = size * 1024;
    } 
    else if (unit == 'M' || unit == 'm') {
        bytes = size * 1024 * 1024;
    } 
    else {
        std::cout << "Unidad no válida\n";
        return;
    }

    std::ofstream file(path, std::ios::binary);

    if (!file) {
        std::cout << "Error creando el archivo\n";
        return;
    }

    // Buffer de 1024 bytes lleno de ceros
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    int remaining = bytes;

    while (remaining > 0) {
        int writeSize = (remaining > 1024) ? 1024 : remaining;
        file.write(buffer, writeSize);
        remaining -= writeSize;
    }

    file.close();
    // Reabrimos el archivo para escribir el MBR
std::fstream disk(path, std::ios::in | std::ios::out | std::ios::binary);

if (!disk) {
    std::cout << "Error abriendo el disco para escribir el MBR\n";
    return;
}

MBR mbr;

mbr.mbr_tamano = bytes;
mbr.mbr_fecha_creacion = time(nullptr);
mbr.mbr_dsk_signature = rand();
mbr.dsk_fit = fit;

// Escribir MBR al inicio del disco
disk.seekp(0);
std::cout << "Bytes calculados: " << bytes << std::endl;

disk.write(reinterpret_cast<char*>(&mbr), sizeof(MBR));

disk.close();

std::cout << "MBR escrito correctamente\n";

    std::cout << "Disco creado correctamente y lleno con ceros\n";
}
