#include <iostream>
#include <cstdlib>
#include <ctime>

#include "commands/mkdisk.h"
#include "commands/fdisk.h"
#include "commands/rmdisk.h"

int main(int argc, char* argv[]) {

srand(time(nullptr));

if (argc < 2) {
std::cout << "Debe ingresar un comando\n";
return 1;
}

std::string command = argv[1];

// ================== MKDISK ==================
if (command == "mkdisk") {

if (argc != 6) {
std::cout << "Uso: ./extreamfs mkdisk <size> <unit> <fit> <path>\n";
return 1;
}

int size = std::stoi(argv[2]);
char unit = argv[3][0];
std::string fit = argv[4];

std::string path = argv[5];

MkDisk mk;
mk.execute(size, unit, fit, path);
}

// ================== FDISK ==================
else if (command == "fdisk") {

if (argc != 8) {
std::cout << "Uso: ./extreamfs fdisk <size> <unit> <path> <type> <fit> <name>\n";
return 1;
}

int size = std::stoi(argv[2]);
char unit = argv[3][0];
std::string path = argv[4];
char type = argv[5][0];
std::string fit = argv[6];
std::string name = argv[7];

FDisk fd;
fd.execute(size, unit, path, type, fit, name);
}

// ================== RMDISK ==================
else if (command == "rmdisk") {

if (argc != 3) {
std::cout << "Uso: ./extreamfs rmdisk <path>\n";
return 1;
}

std::string path = argv[2];

RmDisk rm;
rm.execute(path);
}

else {
std::cout << "Comando no reconocido\n";
}
return 0;
}