#include <iostream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include "mount/mount_manager.h"
#include "commands/mkdisk.h"
#include "commands/fdisk.h"
#include "commands/rmdisk.h"
#include "commands/mount.h"

std::vector<std::string> split(std::string input) {
    std::stringstream ss(input);
    std::string word;
    std::vector<std::string> tokens;

    while (ss >> word) {
        tokens.push_back(word);
    }

    return tokens;
}

int main() {

    srand(time(nullptr));

    std::string input;

    std::cout << "====== EXTREAMFS CONSOLA ======\n";
    std::cout << "Escriba 'exit' para salir\n\n";

    while (true) {

        std::cout << "extreamfs> ";
        std::getline(std::cin, input);

        if (input.empty()) continue;

        if (input == "exit") {
            std::cout << "Saliendo...\n";
            break;
        }

        std::vector<std::string> args = split(input);

        std::string command = args[0];

        // ================== MKDISK ==================
        if (command == "mkdisk") {

            if (args.size() != 5) {
                std::cout << "Uso: mkdisk <size> <unit> <fit> <path>\n";
                continue;
            }

            int size = std::stoi(args[1]);
            char unit = args[2][0];
            std::string fit = args[3];
            std::string path = args[4];

            MkDisk mk;
            mk.execute(size, unit, fit, path);
        }

        // ================== FDISK ==================
        else if (command == "fdisk") {

            if (args.size() != 7) {
                std::cout << "Uso: fdisk <size> <unit> <path> <type> <fit> <name>\n";
                continue;
            }

            int size = std::stoi(args[1]);
            char unit = args[2][0];
            std::string path = args[3];
            char type = args[4][0];
            std::string fit = args[5];
            std::string name = args[6];

            FDisk fd;
            fd.execute(size, unit, path, type, fit, name);
        }

        // ================== RMDISK ==================
        else if (command == "rmdisk") {

            if (args.size() != 2) {
                std::cout << "Uso: rmdisk <path>\n";
                continue;
            }

            std::string path = args[1];

            RmDisk rm;
            rm.execute(path);
        }

        // ================== MOUNT ==================
        else if (command == "mount") {

            if (args.size() == 1) {
                // Solo listar montadas
                MountManager::showMounted();
                continue;
            }

            if (args.size() != 3) {
                std::cout << "Uso: mount <path> <name>\n";
                continue;
            }

            std::string path = args[1];
            std::string name = args[2];

            Mount m;
            m.execute(path, name);
        }

        else {
            std::cout << "Comando no reconocido\n";
        }
    }

    return 0;
}
