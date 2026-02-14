#include "commands/mkdisk.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include "disk/mbr.h"
#include <ctime>
#include <cstdlib>

void MkDisk::execute(int size, char unit, std::string fit, std::string path) {

    if (size <= 0) {
        std::cout << "ERROR: El tamaño debe ser mayor a 0\n";
        return;
    }

    // ✅ Validación correcta para 2 letras
    if (fit != "BF" && fit != "FF" && fit != "WF") {
        std::cout << "ERROR: Fit inválido (use BF, FF o WF)\n";
        return;
    }

    int bytes = 0;

    if (unit == 'K' || unit == 'k')
        bytes = size * 1024;
    else if (unit == 'M' || unit == 'm')
        bytes = size * 1024 * 1024;
    else {
        std::cout << "ERROR: Unidad no válida\n";
        return;
    }

    std::ofstream file(path, std::ios::binary);

    if (!file) {
        std::cout << "Error creando el archivo\n";
        return;
    }

    char buffer[1024] = {0};

    int remaining = bytes;

    while (remaining > 0) {
        int writeSize = (remaining > 1024) ? 1024 : remaining;
        file.write(buffer, writeSize);
        remaining -= writeSize;
    }

    file.close();

    std::fstream disk(path, std::ios::in | std::ios::out | std::ios::binary);

    if (!disk) {
        std::cout << "Error abriendo el disco para escribir el MBR\n";
        return;
    }

    MBR mbr;
    memset(&mbr, 0, sizeof(MBR));

    mbr.mbr_tamano = bytes;
    mbr.mbr_fecha_creacion = time(nullptr);
    mbr.mbr_dsk_signature = rand();

    // ✅ Copiar correctamente el fit al MBR
    strncpy(mbr.dsk_fit, fit.c_str(), 2);
    mbr.dsk_fit[2] = '\0';

    // Inicializar particiones
    for (int i = 0; i < 4; i++) {
        mbr.mbr_partitions[i].part_status = '0';
        mbr.mbr_partitions[i].part_type = '0';
        memset(mbr.mbr_partitions[i].part_fit, 0, 3);
        mbr.mbr_partitions[i].part_start = -1;
        mbr.mbr_partitions[i].part_size = 0;
        memset(mbr.mbr_partitions[i].part_name, 0, 16);
    }

    disk.seekp(0);
    disk.write(reinterpret_cast<char*>(&mbr), sizeof(MBR));
    disk.close();

    std::cout << "Disco creado correctamente\n";
}
