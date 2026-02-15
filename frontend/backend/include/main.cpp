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
#include "commands/mkfs.h"

#include <algorithm>

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

    int size = -1;
    char unit = 'M';          // default
    std::string fit = "FF";   // default
    std::string path = "";

    for (int i = 1; i < args.size(); i++) {

        std::string param = args[i];

        // Convertir a minúsculas para comparar
        std::string lower = param;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        if (lower.find("-size=") == 0) {
            size = std::stoi(param.substr(6));
        }
        else if (lower.find("-unit=") == 0) {
            unit = toupper(param.substr(6)[0]);
        }
        else if (lower.find("-fit=") == 0) {
            fit = param.substr(5);
            std::transform(fit.begin(), fit.end(), fit.begin(), ::toupper);
        }
        else if (lower.find("-path=") == 0) {
            path = param.substr(6);
        }
    }

    // ===== VALIDACIONES =====

    if (size <= 0) {
        std::cout << "ERROR: size debe ser mayor a 0\n";
        continue;
    }

    if (unit != 'K' && unit != 'M') {
        std::cout << "ERROR: unit debe ser K o M\n";
        continue;
    }

    if (fit != "BF" && fit != "FF" && fit != "WF") {
        std::cout << "ERROR: fit debe ser BF, FF o WF\n";
        continue;
    }

    if (path.empty()) {
        std::cout << "ERROR: path es obligatorio\n";
        continue;
    }

    MkDisk mk;
    mk.execute(size, unit, fit, path);
}


      // ================== FDISK ==================
else if (command == "fdisk") {

    int size = -1;
    char unit = 'K';            // Default según enunciado
    std::string path = "";
    char type = 'P';            // Default primaria
    std::string fit = "WF";     // Default peor ajuste
    std::string name = "";

    for (int i = 1; i < args.size(); i++) {

        std::string param = args[i];
        std::string lower = param;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        if (lower.find("-size=") == 0) {
            size = std::stoi(param.substr(6));
        }
        else if (lower.find("-unit=") == 0) {
            unit = toupper(param.substr(6)[0]);
        }
        else if (lower.find("-path=") == 0) {
            path = param.substr(6);
        }
        else if (lower.find("-type=") == 0) {
            type = toupper(param.substr(6)[0]);
        }
        else if (lower.find("-fit=") == 0) {
            fit = param.substr(5);
            std::transform(fit.begin(), fit.end(), fit.begin(), ::toupper);
        }
        else if (lower.find("-name=") == 0) {
            name = param.substr(6);
        }
    }

    // ===== VALIDACIONES MÍNIMAS DE ESTRUCTURA =====

    if (size <= 0) {
        std::cout << "ERROR: El parámetro -size es obligatorio y debe ser mayor a 0\n";
        continue;
    }

    if (path.empty()) {
        std::cout << "ERROR: El parámetro -path es obligatorio\n";
        continue;
    }

    if (name.empty()) {
        std::cout << "ERROR: El parámetro -name es obligatorio\n";
        continue;
    }

    // El resto de validaciones las hace fdisk.cpp

    FDisk fd;
    fd.execute(size, unit, path, type, fit, name);
}


        // ================== RMDISK ==================
        else if (command == "rmdisk") {

            std::string path = "";

            for (int i = 1; i < args.size(); i++) {

                std::string param = args[i];
                std::string lower = param;
                std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

                if (lower.find("-path=") == 0) {
                    path = param.substr(6);
                }
            }

            // ===== VALIDACIONES =====
            if (path.empty()) {
                std::cout << "ERROR: El parámetro -path es obligatorio\n";
                continue;
            }

            RmDisk rm;
            rm.execute(path);
        }


        // ================== MOUNT ==================
                else if (command == "mount") {

            if (args.size() == 1) {
                MountManager::showMounted();
                continue;
            }

            std::string path = "";
            std::string name = "";

            for (int i = 1; i < args.size(); i++) {

                if (args[i].find("-path=") == 0) {
                    path = args[i].substr(6);
                }
                else if (args[i].find("-name=") == 0) {
                    name = args[i].substr(6);
                }
            }

            if (path.empty() || name.empty()) {
                std::cout << "Uso: mount -path=<ruta> -name=<nombre>\n";
                continue;
            }

            Mount m;
            m.execute(path, name);
        }
        // ================== MKFS ==================
else if (command == "mkfs") {

    std::string id = "";
    std::string type = "full"; // default

    for (int i = 1; i < args.size(); i++) {

        std::string param = args[i];
        std::string lower = param;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        if (lower.find("-id=") == 0) {
            id = param.substr(4);
        }
        else if (lower.find("-type=") == 0) {
            type = lower.substr(6);
        }
    }

    // ===== VALIDACIONES =====

    if (id.empty()) {
        std::cout << "ERROR: El parámetro -id es obligatorio\n";
        continue;
    }

    if (type != "full") {
        std::cout << "ERROR: Solo se permite -type=full\n";
        continue;
    }

    Mkfs mkfs;
    mkfs.execute(id);
}



        else {
            std::cout << "Comando no reconocido\n";
        }
    }

    return 0;
}
