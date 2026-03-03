#include "fdisk.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include "../disk/mbr.h"
#include "../disk/ebr.h"
#include <vector>
#include <algorithm>
#include <climits>

// ✅ Fix #2: long long para offsets
struct EspacioLibre {
    long long inicio;
    long long tamano;
};

// ✅ Fix #3: long long en pos
bool FDisk::existeNombreLogica(FILE* disk, long long startExtendida, std::string nombre) {

    EBR ebr;
    long long pos = startExtendida;

    while (pos != -1) {
        fseek(disk, pos, SEEK_SET);
        fread(&ebr, sizeof(EBR), 1, disk);

        if (ebr.part_size > 0) {
            if (strcmp(ebr.part_name, nombre.c_str()) == 0)
                return true;
        }

        pos = ebr.part_next;
    }

    return false;
}

void FDisk::printLogicas(FILE* disk, long long startExtendida) {

    EBR ebr;
    long long pos = startExtendida;
    int contador = 0;

    std::cout << "\n----- PARTICIONES LOGICAS -----\n";

    while (pos != -1) {
        fseek(disk, pos, SEEK_SET);
        fread(&ebr, sizeof(EBR), 1, disk);

        if (ebr.part_size > 0) {
            std::cout << "EBR #" << contador << "\n";
            std::cout << "  Name  : " << ebr.part_name  << "\n";
            std::cout << "  Start : " << ebr.part_start << "\n";
            std::cout << "  Size  : " << ebr.part_size  << "\n";
            std::cout << "  Next  : " << ebr.part_next  << "\n";
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

    // ✅ Fix #5: validar fit ANTES de usarlo
    if (fit.empty()) fit = "WF";
    if (fit != "BF" && fit != "FF" && fit != "WF") {
        std::cout << "ERROR: Fit inválido (use BF, FF o WF)\n";
        return;
    }

    // ✅ Fix #1: long long para bytes
    long long bytes = size;
    if (unit == 'K' || unit == 'k')
        bytes *= 1024;
    else if (unit == 'M' || unit == 'm')
        bytes *= 1024 * 1024;
    else if (unit != 'B' && unit != 'b') {
        std::cout << "ERROR: Unidad inválida\n";
        return;
    }

    FILE* file = fopen(path.c_str(), "rb+");
    if (!file) {
        std::cout << "ERROR: No se pudo abrir el disco\n";
        return;
    }

    MBR mbr;
    fseek(file, 0, SEEK_SET);
    fread(&mbr, sizeof(MBR), 1, file);

    // Validar type
    if (type != 'P' && type != 'E' && type != 'L') {
        std::cout << "ERROR: Tipo de partición inválido\n";
        fclose(file);
        return;
    }

    // Contar particiones
    int primaryExtendedCount = 0;
    bool extendedExists = false;

    for (int i = 0; i < 4; i++) {
        if (mbr.mbr_partitions[i].part_status == '1') {
            if (mbr.mbr_partitions[i].part_type == 'P' ||
                mbr.mbr_partitions[i].part_type == 'E') {
                primaryExtendedCount++;
                if (mbr.mbr_partitions[i].part_type == 'E')
                    extendedExists = true;
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

    // ================= PARTICIÓN LÓGICA =================
    if (type == 'L') {

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

        long long inicio_ext = extendedPartition.part_start;
        long long fin_ext    = extendedPartition.part_start + extendedPartition.part_size;

        if (existeNombreLogica(file, inicio_ext, name)) {
            std::cout << "ERROR: Ya existe una partición lógica con ese nombre\n";
            fclose(file);
            return;
        }

        long long ebrPosition = inicio_ext;
        EBR ebr;
        fseek(file, ebrPosition, SEEK_SET);
        fread(&ebr, sizeof(EBR), 1, file);

        // Primera lógica
        if (ebr.part_start == -1) {

            long long logicStart = ebrPosition + sizeof(EBR);

            if (logicStart + bytes > fin_ext) {
                std::cout << "ERROR: No hay espacio dentro de la extendida\n";
                fclose(file);
                return;
            }

            ebr.part_mount = '0';
            // ✅ Fix #4: usar strncpy para part_fit[3]
            strncpy(ebr.part_fit, fit.c_str(), 2);
            ebr.part_fit[2] = '\0';
            ebr.part_start  = logicStart;
            ebr.part_size   = bytes;
            ebr.part_next   = -1;
            memset(ebr.part_name, 0, 16);
            strncpy(ebr.part_name, name.c_str(), 15);

            fseek(file, ebrPosition, SEEK_SET);
            fwrite(&ebr, sizeof(EBR), 1, file);

            printLogicas(file, inicio_ext);
            fclose(file);
            std::cout << "OK: Partición lógica creada correctamente\n";
            return;
        }

        // Agregar al final
        long long currentEBRPos = inicio_ext;
        EBR currentEBR;

        while (true) {
            fseek(file, currentEBRPos, SEEK_SET);
            fread(&currentEBR, sizeof(EBR), 1, file);
            if (currentEBR.part_next == -1) break;
            currentEBRPos = currentEBR.part_next;
        }

        long long newEBRPos    = currentEBR.part_start + currentEBR.part_size;
        long long newLogicStart = newEBRPos + sizeof(EBR);

        if (newLogicStart + bytes > fin_ext) {
            std::cout << "ERROR: No hay espacio suficiente dentro de la extendida\n";
            fclose(file);
            return;
        }

        currentEBR.part_next = newEBRPos;
        fseek(file, currentEBRPos, SEEK_SET);
        fwrite(&currentEBR, sizeof(EBR), 1, file);

        EBR newEBR;
        memset(&newEBR, 0, sizeof(EBR));
        newEBR.part_mount = '0';
        strncpy(newEBR.part_fit, fit.c_str(), 2);
        newEBR.part_fit[2]  = '\0';
        newEBR.part_start   = newLogicStart;
        newEBR.part_size    = bytes;
        newEBR.part_next    = -1;
        memset(newEBR.part_name, 0, 16);
        strncpy(newEBR.part_name, name.c_str(), 15);

        fseek(file, newEBRPos, SEEK_SET);
        fwrite(&newEBR, sizeof(EBR), 1, file);

        printLogicas(file, inicio_ext);
        fclose(file);
        std::cout << "OK: Partición lógica creada correctamente\n";
        return;
    }

    // ================= VALIDAR NOMBRE DUPLICADO =================
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

    // ✅ Fix #6: long long para diskStart y diskEnd
    long long diskStart = sizeof(MBR);
    long long diskEnd   = mbr.mbr_tamano;

    std::vector<Partition> activas;
    for (int i = 0; i < 4; i++) {
        if (mbr.mbr_partitions[i].part_status == '1')
            activas.push_back(mbr.mbr_partitions[i]);
    }

    std::sort(activas.begin(), activas.end(),
        [](Partition a, Partition b) {
            return a.part_start < b.part_start;
        });

    long long current = diskStart;
    for (auto &p : activas) {
        if (p.part_start > current) {
            libres.push_back({current, p.part_start - current});
        }
        current = p.part_start + p.part_size;
    }
    if (current < diskEnd)
        libres.push_back({current, diskEnd - current});

    // ================= APLICAR FIT =================
    long long start = -1;

    if (fit == "FF") {
        for (auto &e : libres) {
            if (e.tamano >= bytes) { start = e.inicio; break; }
        }
    } else if (fit == "BF") {
        long long mejor = LLONG_MAX;
        for (auto &e : libres) {
            if (e.tamano >= bytes && e.tamano < mejor) {
                mejor = e.tamano;
                start = e.inicio;
            }
        }
    } else if (fit == "WF") {
        long long peor = -1;
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

    // ================= CREAR PARTICIÓN =================
    Partition newPartition;
    memset(&newPartition, 0, sizeof(Partition));
    newPartition.part_status = '1';
    newPartition.part_type   = type;
    strncpy(newPartition.part_fit, fit.c_str(), 2);
    newPartition.part_fit[2] = '\0';
    newPartition.part_start  = start;
    newPartition.part_size   = bytes;
    memset(newPartition.part_name, 0, 16);
    strncpy(newPartition.part_name, name.c_str(), 15);

    mbr.mbr_partitions[index] = newPartition;

    fseek(file, 0, SEEK_SET);
    fwrite(&mbr, sizeof(MBR), 1, file);

    // ================= SI ES EXTENDIDA: PRIMER EBR =================
    if (type == 'E') {
        EBR firstEBR;
        memset(&firstEBR, 0, sizeof(EBR));
        firstEBR.part_mount  = '0';
        strncpy(firstEBR.part_fit, "0", 1);
        firstEBR.part_start  = -1;
        firstEBR.part_size   = 0;
        firstEBR.part_next   = -1;
        memset(firstEBR.part_name, 0, 16);

        fseek(file, start, SEEK_SET);
        fwrite(&firstEBR, sizeof(EBR), 1, file);
    }

    fclose(file);
    std::cout << "OK: Partición '" << name << "' creada correctamente\n";
}