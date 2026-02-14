
#include "fdisk.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include "../disk/mbr.h"

void FDisk::execute(int size, char unit, std::string path,
                    char type, char fit, std::string name) {

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
    fclose(file);
    return;
}

// Abrir disco
FILE* file = fopen(path.c_str(), "rb+");

if (!file) {
std::cout << "ERROR: No se pudo abrir el disco\n";
return;
}

// Leer MBR
MBR mbr;
fseek(file, 0, SEEK_SET);
fread(&mbr, sizeof(MBR), 1, file);

// ================= VALIDACIONES TYPE Y RESTRICCIONES =================

// Validar tipo
if (type != 'P' && type != 'E' && type != 'L') {
std::cout << "ERROR: Tipo de partición inválido\n";
fclose(file);
return;
}

// Contar primarias y extendidas
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

// Si intenta crear extendida y ya existe
if (type == 'E' && extendedExists) {
std::cout << "ERROR: Ya existe una partición extendida\n";
fclose(file);
return;
}

// Si ya hay 4 primarias/extendidas
if ((type == 'P' || type == 'E') && primaryExtendedCount >= 4) {
std::cout << "ERROR: Límite de 4 particiones primarias/extendidas alcanzado\n";
fclose(file);
return;
}

// Si intenta crear lógica sin extendida
if (type == 'L' && !extendedExists) {
std::cout << "ERROR: No existe partición extendida para crear lógica\n";
fclose(file);
return;
}

// Validar nombre repetido
for (int i = 0; i < 4; i++) {
if (mbr.mbr_partitions[i].part_status == '1') {
if (strncmp(mbr.mbr_partitions[i].part_name, name.c_str(), 16) == 0) {
std::cout << "ERROR: Ya existe una partición con ese nombre\n";
fclose(file);
return;
}
}
}

// Buscar espacio libre en tabla de particiones
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

// Calcular start (First Fit simple)
int start = sizeof(MBR);

for (int i = 0; i < index; i++) {
if (mbr.mbr_partitions[i].part_status == '1') {
start = mbr.mbr_partitions[i].part_start +
mbr.mbr_partitions[i].part_size;
}
}

// Verificar espacio suficiente
if (start + bytes > mbr.mbr_tamano) {
std::cout << "ERROR: No hay espacio suficiente en el disco\n";
fclose(file);
return;
}
// ================= VALIDAR FIT =================

if (fit.empty()) {
    fit = "WF";
}

if (fit != "BF" && fit != "FF" && fit != "WF") {
std::cout << "ERROR: Fit inválido (use BF, FF o WF)\n";
fclose(file);
return;
}

// Crear partición
Partition newPartition;

newPartition.part_status = '1';
newPartition.part_type = type; // 'P'
memset(newPartition.part_fit, 0, 3);
strncpy(newPartition.part_fit, fit.c_str(), 2);//validación fit

newPartition.part_start = start;
newPartition.part_size = bytes;

memset(newPartition.part_name, 0, 16);
strncpy(newPartition.part_name, name.c_str(), 15);

// Guardar en MBR
mbr.mbr_partitions[index] = newPartition;

// Reescribir MBR
fseek(file, 0, SEEK_SET);
fwrite(&mbr, sizeof(MBR), 1, file);

fclose(file);
std::cout << "Partición creada correctamente\n";
}
