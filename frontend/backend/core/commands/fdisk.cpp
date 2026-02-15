#include "fdisk.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include "../disk/mbr.h"
#include "../disk/ebr.h"
#include <vector>
#include <algorithm>
#include <climits>


struct EspacioLibre {
    int inicio;
    int tamano;
};

bool FDisk::existeNombreLogica(FILE* disk, int startExtendida, std::string nombre) {

    EBR ebr;
    int pos = startExtendida;

    while (pos != -1) {

        fseek(disk, pos, SEEK_SET);
        fread(&ebr, sizeof(EBR), 1, disk);

        if (ebr.part_s > 0) {
            if (strcmp(ebr.part_name, nombre.c_str()) == 0) {
                return true;
            }
        }

        pos = ebr.part_next;
    }

    return false;
}

void FDisk::printLogicas(FILE* disk, int startExtendida) {

    EBR ebr;
    int pos = startExtendida;
    int contador = 0;

    std::cout << "\n----- PARTICIONES LOGICAS -----\n";

while (pos != -1) {

    fseek(disk, pos, SEEK_SET);
    fread(&ebr, sizeof(EBR), 1, disk);

    if (ebr.part_s > 0) {

        std::cout << "EBR #" << contador << std::endl;
        std::cout << "  Name  : " << ebr.part_name << std::endl;
        std::cout << "  Start : " << ebr.part_start << std::endl;
        std::cout << "  Size  : " << ebr.part_s << std::endl;
        std::cout << "  Next  : " << ebr.part_next << std::endl;
        std::cout << "---------------------------\n";
        contador++;
    }

        pos = ebr.part_next;
    }

    if (contador == 0)
        std::cout << "No hay particiones logicas creadas.\n";
}

void FDisk::execute(int size, char unit, std::string path,
                    char type, std::string fit, std::string name) {

    if (size <= 0) {
        std::cout << "ERROR: El tamaño debe ser mayor a 0\n";
        return;
    }

    // Convertir a bytes
    int bytes = size;

    if (unit == 'K' || unit == 'k')
        bytes *= 1024;
    else if (unit == 'M' || unit == 'm')
        bytes *= 1024 * 1024;
    else if (unit != 'B' && unit != 'b') {
        std::cout << "ERROR: Unidad inválida\n";
        return;   // ❌ Ya NO cerramos nada aquí
    }

    // ================= ABRIR DISCO =================
    FILE* file = fopen(path.c_str(), "rb+");

    if (!file) {
        std::cout << "ERROR: No se pudo abrir el disco\n";
        return;
    }

    // Leer MBR
    MBR mbr;
    fseek(file, 0, SEEK_SET);
    fread(&mbr, sizeof(MBR), 1, file);

//     std::cout << "\n----- DEBUG MBR -----\n";
//     for (int i = 0; i < 4; i++) {
//         if (mbr.mbr_partitions[i].part_status == '1') {
//             std::cout << "Particion " << i << "\n";
//             std::cout << "Type: " << mbr.mbr_partitions[i].part_type << "\n";
//             std::cout << "Start: " << mbr.mbr_partitions[i].part_start << "\n";
//             std::cout << "Size: " << mbr.mbr_partitions[i].part_size << "\n";
//             std::cout << "Name: " << mbr.mbr_partitions[i].part_name << "\n\n";
//         }
// }


    // ================= VALIDAR TYPE =================
    if (type != 'P' && type != 'E' && type != 'L') {
        std::cout << "ERROR: Tipo de partición inválido\n";
        fclose(file);
        return;
    }

    // ================= CONTAR PARTICIONES =================
    int primaryExtendedCount = 0;
    bool extendedExists = false;

    for (int i = 0; i < 4; i++) {
        if (mbr.mbr_partitions[i].part_status == '1') {

            if (mbr.mbr_partitions[i].part_type == 'P' ||
                mbr.mbr_partitions[i].part_type == 'E') {

                primaryExtendedCount++;

                if (mbr.mbr_partitions[i].part_type == 'E') {
                    extendedExists = true;
                }
            }
        }
    }

    if (type == 'E' && extendedExists) {
        std::cout << "ERROR: Ya existe una partición extendida\n";
        fclose(file);
        return;
    }

    if ((type == 'P' || type == 'E') && primaryExtendedCount >= 4) {
        std::cout << "ERROR: Límite de 4 particiones alcanzado\n";
        fclose(file);
        return;
    }

    if (type == 'L' && !extendedExists) {
        std::cout << "ERROR: No existe partición extendida\n";
        fclose(file);
        return;
    }

    // ================= CREAR PARTICIÓN LÓGICA (PRIMER CASO) =================
    if (type == 'L') {
 // Buscar partición extendida
    Partition extendedPartition;
    bool found = false;

    for (int i = 0; i < 4; i++) {
        if (mbr.mbr_partitions[i].part_status == '1' &&
            mbr.mbr_partitions[i].part_type == 'E') {

            extendedPartition = mbr.mbr_partitions[i];
            found = true;
            break;
        }
    }

    if (!found) {
        std::cout << "ERROR: No se encontró la partición extendida\n";
        fclose(file);
        return;
    }

    // Límites reales de la extendida
    int inicio_ext = extendedPartition.part_start;
    int fin_ext = extendedPartition.part_start + extendedPartition.part_size;

    // Validar nombre duplicado
    if (existeNombreLogica(file, inicio_ext, name)) {
        std::cout << "ERROR: Ya existe una partición lógica con ese nombre\n";
        fclose(file);
        return;
    }

    // Leer primer EBR
    int ebrPosition = inicio_ext;
    EBR ebr;

    fseek(file, ebrPosition, SEEK_SET);
    fread(&ebr, sizeof(EBR), 1, file);

    // ================= PRIMERA LÓGICA =================
    if (ebr.part_start == -1) {

        int logicStart = ebrPosition + sizeof(EBR);

        if (logicStart + bytes > fin_ext) {
            std::cout << "ERROR: No hay espacio dentro de la extendida\n";
            fclose(file);
            return;
        }

        ebr.part_mount = '0';
        ebr.part_fit = fit[0];
        ebr.part_start = logicStart;
        ebr.part_s = bytes;
        ebr.part_next = -1;

        memset(ebr.part_name, 0, 16);
        strncpy(ebr.part_name, name.c_str(), 15);

        fseek(file, ebrPosition, SEEK_SET);
        fwrite(&ebr, sizeof(EBR), 1, file);

        printLogicas(file, inicio_ext);
        fclose(file);
        std::cout << "Partición lógica creada correctamente\n";
        return;
    }

    // ================= AGREGAR AL FINAL =================

    int currentEBRPos = inicio_ext;
    EBR currentEBR;

    while (true) {
        fseek(file, currentEBRPos, SEEK_SET);
        fread(&currentEBR, sizeof(EBR), 1, file);

        if (currentEBR.part_next == -1) {
            break;
        }

        currentEBRPos = currentEBR.part_next;
    }

    // Posición del nuevo EBR
    int newEBRPos = currentEBR.part_start + currentEBR.part_s;
    int newLogicStart = newEBRPos + sizeof(EBR);

    // VALIDACIÓN CRÍTICA
    if (newLogicStart + bytes > fin_ext) {
        std::cout << "ERROR: No hay espacio suficiente dentro de la extendida\n";
        fclose(file);
        return;
    }

    // Actualizar next del último
    currentEBR.part_next = newEBRPos;
    fseek(file, currentEBRPos, SEEK_SET);
    fwrite(&currentEBR, sizeof(EBR), 1, file);

    // Crear nuevo EBR
    EBR newEBR;
    newEBR.part_mount = '0';
    newEBR.part_fit = fit[0];
    newEBR.part_start = newLogicStart;
    newEBR.part_s = bytes;
    newEBR.part_next = -1;

    memset(newEBR.part_name, 0, 16);
    strncpy(newEBR.part_name, name.c_str(), 15);

    fseek(file, newEBRPos, SEEK_SET);
    fwrite(&newEBR, sizeof(EBR), 1, file);

    printLogicas(file, inicio_ext);
    fclose(file);
    std::cout << "Partición lógica creada correctamente\n";
    return;
    }


    // ================= VALIDAR NOMBRE =================
    for (int i = 0; i < 4; i++) {
        if (mbr.mbr_partitions[i].part_status == '1') {
            if (strncmp(mbr.mbr_partitions[i].part_name, name.c_str(), 16) == 0) {
                std::cout << "ERROR: Ya existe una partición con ese nombre\n";
                fclose(file);
                return;
            }
        }
    }

    // ================= BUSCAR SLOT LIBRE =================
    int index = -1;

    for (int i = 0; i < 4; i++) {
        if (mbr.mbr_partitions[i].part_status == '0') {
            index = i;
            break;
        }
    }

    if (index == -1) {
        std::cout << "ERROR: No hay espacio en la tabla de particiones\n";
        fclose(file);
        return;
    }

        // ================= CALCULAR ESPACIOS LIBRES =================
    std::vector<EspacioLibre> libres;

    int diskStart = sizeof(MBR);
    int diskEnd = mbr.mbr_tamano;

    // 1. Obtener particiones activas
    std::vector<Partition> activas;

    for (int i = 0; i < 4; i++) {
        if (mbr.mbr_partitions[i].part_status == '1') {
            activas.push_back(mbr.mbr_partitions[i]);
        }
    }

    // 2. Ordenarlas por start
    std::sort(activas.begin(), activas.end(),
            [](Partition a, Partition b) {
                return a.part_start < b.part_start;
            });

    // 3. Detectar huecos
    int current = diskStart;

    for (auto &p : activas) {
        if (p.part_start > current) {
            EspacioLibre e;
            e.inicio = current;
            e.tamano = p.part_start - current;
            libres.push_back(e);
        }
        current = p.part_start + p.part_size;
    }

    // 4. Hueco final
    if (current < diskEnd) {
        EspacioLibre e;
        e.inicio = current;
        e.tamano = diskEnd - current;
        libres.push_back(e);
    }
    int start = -1;

for (auto &e : libres) {
    if (e.tamano >= bytes) {

        if (fit == "FF") {
            start = e.inicio;
            break;
        }
    }
}

if (fit == "BF") {
    int mejor = INT_MAX;
    for (auto &e : libres) {
        if (e.tamano >= bytes && e.tamano < mejor) {
            mejor = e.tamano;
            start = e.inicio;
        }
    }
}

if (fit == "WF") {
    int peor = -1;
    for (auto &e : libres) {
        if (e.tamano >= bytes && e.tamano > peor) {
            peor = e.tamano;
            start = e.inicio;
        }
    }
}

if (start == -1) {
    std::cout << "ERROR: No hay espacio suficiente en el disco\n";
    fclose(file);
    return;
}


    // ================= VALIDAR FIT =================
    if (fit.empty())
        fit = "WF";

    if (fit != "BF" && fit != "FF" && fit != "WF") {
        std::cout << "ERROR: Fit inválido (use BF, FF o WF)\n";
        fclose(file);
        return;
    }

    // ================= CREAR PARTICIÓN =================
    Partition newPartition;
    memset(&newPartition, 0, sizeof(Partition));
    newPartition.part_status = '1';
    newPartition.part_type = type;
    strncpy(newPartition.part_fit, fit.c_str(), 2);


    newPartition.part_start = start;
    newPartition.part_size = bytes;

    memset(newPartition.part_name, 0, 16);
    strncpy(newPartition.part_name, name.c_str(), 15);

    // Guardar en MBR
    mbr.mbr_partitions[index] = newPartition;

    // Reescribir MBR
    fseek(file, 0, SEEK_SET);
    fwrite(&mbr, sizeof(MBR), 1, file);

    // ================= SI ES EXTENDIDA CREAR PRIMER EBR =================
if (type == 'E') {

    EBR firstEBR;
    memset(&firstEBR, 0, sizeof(EBR));

    firstEBR.part_mount = '0';
    firstEBR.part_fit = '0';
    firstEBR.part_start = -1;
    firstEBR.part_s = 0;
    firstEBR.part_next = -1;
    memset(firstEBR.part_name, 0, 16);

    // Escribir EBR al inicio de la extendida
    fseek(file, start, SEEK_SET);
    fwrite(&firstEBR, sizeof(EBR), 1, file);
}


    fclose(file);   

    std::cout << "Partición creada correctamente\n";
}
