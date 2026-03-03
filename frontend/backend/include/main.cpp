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
#include "filesystem/login.h"
#include "filesystem/logout.h"
#include "commands/mkgrp.h"
#include "commands/rmgrp.h"
#include "commands/mkusr.h"
#include "commands/rmusr.h"
#include "commands/chgrp.h"
#include "commands/mkdir_cmd.h"
#include "commands/mkfile_cmd.h"
#include "commands/cat_cmd.h"
#include "commands/rep_cmd.h"
#include <algorithm>
#include <string>

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

            //  Salir si EOF (cuando se usa < archivo.txt)
            if (std::cin.eof()) break;

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


        // ================== LOGIN ==================
        else if (command == "login") {
            std::string user = "", pass = "", id = "";

            for (int i = 1; i < args.size(); i++) {
                std::string p = args[i];
                std::string lower = p;
                transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

                if (lower.find("-user=") == 0) user = p.substr(6);
                else if (lower.find("-pass=") == 0) pass = p.substr(6);
                else if (lower.find("-id=")   == 0) id   = p.substr(4);
            }

            if (user.empty() || pass.empty() || id.empty()) {
                std::cout << "Uso: login -user=<usuario> -pass=<contraseña> -id=<id>\n";
                continue;
            }

            Login l;
            l.execute(user, pass, id);
        }

        // ================== LOGOUT ==================
        else if (command == "logout") {
            Logout lo;
            lo.execute();
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
        // ================== MKGRP ==================
else if (command == "mkgrp") {
    std::string name = "";
    for (int i = 1; i < args.size(); i++) {
        std::string lower = args[i];
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        if (lower.find("-name=") == 0) name = args[i].substr(6);
    }
    if (name.empty()) { std::cout << "ERROR: -name es obligatorio\n"; continue; }
    MkGrp mg; mg.execute(name);
}

// ================== RMGRP ==================
else if (command == "rmgrp") {
    std::string name = "";
    for (int i = 1; i < args.size(); i++) {
        std::string lower = args[i];
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        if (lower.find("-name=") == 0) name = args[i].substr(6);
    }
    if (name.empty()) { std::cout << "ERROR: -name es obligatorio\n"; continue; }
    RmGrp rg; rg.execute(name);
}

// ================== MKUSR ==================
else if (command == "mkusr") {
    std::string user = "", pass = "", grp = "";
    for (int i = 1; i < args.size(); i++) {
        std::string p = args[i];
        std::string lower = p;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        if (lower.find("-user=") == 0) user = p.substr(6);
        else if (lower.find("-pass=") == 0) pass = p.substr(6);
        else if (lower.find("-grp=")  == 0) grp  = p.substr(5);
    }
    if (user.empty() || pass.empty() || grp.empty()) {
        std::cout << "ERROR: -user, -pass y -grp son obligatorios\n"; continue;
    }
    MkUsr mu; mu.execute(user, pass, grp);
}

// ================== RMUSR ==================
else if (command == "rmusr") {
    std::string user = "";
    for (int i = 1; i < args.size(); i++) {
        std::string lower = args[i];
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        if (lower.find("-user=") == 0) user = args[i].substr(6);
    }
    if (user.empty()) { std::cout << "ERROR: -user es obligatorio\n"; continue; }
    RmUsr ru; ru.execute(user);
}

// ================== CHGRP ==================
else if (command == "chgrp") {
    std::string user = "", grp = "";
    for (int i = 1; i < args.size(); i++) {
        std::string p = args[i];
        std::string lower = p;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        if (lower.find("-user=") == 0) user = p.substr(6);
        else if (lower.find("-grp=") == 0) grp = p.substr(5);
    }
    if (user.empty() || grp.empty()) {
        std::cout << "ERROR: -user y -grp son obligatorios\n"; continue;
    }
    ChGrp cg; cg.execute(user, grp);
}

        // ================== MKDIR ==================
else if (command == "mkdir") {
    std::string path = "";
    bool createParents = false;

    for (int i = 1; i < args.size(); i++) {
        std::string p = args[i];
        std::string lower = p;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        if (lower == "-p") {
            createParents = true;
        }
        else if (lower.find("-path=") == 0) {
            path = p.substr(6);
        }
    }

    if (path.empty()) {
        std::cout << "ERROR: -path es obligatorio\n";
        continue;
    }

    MkdirCmd md;
    md.execute(path, createParents);
}

// ================== MKFILE ==================
else if (command == "mkfile") {
    std::string path = "";
    std::string cont = "";
    int    size = 0;
    bool   createParents = false;

    for (int i = 1; i < args.size(); i++) {
        std::string p = args[i];
        std::string lower = p;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        if (lower == "-r") {
            createParents = true;
        }
        else if (lower.find("-path=") == 0) {
            path = p.substr(6);
        }
        else if (lower.find("-size=") == 0) {
            size = std::stoi(p.substr(6));
            if (size < 0) {
                std::cout << "ERROR: -size no puede ser negativo\n";
                size = 0;
            }
        }
        else if (lower.find("-cont=") == 0) {
            cont = p.substr(6);
        }
    }

    if (path.empty()) {
        std::cout << "ERROR: -path es obligatorio\n";
        continue;
    }

    MkfileCmd mf;
    mf.execute(path, createParents, size, cont);
}

// ================== CAT ==================
else if (command == "cat") {
    std::vector<std::string> files;

    for (int i = 1; i < args.size(); i++) {
        std::string p = args[i];
        std::string lower = p;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        // Acepta -file1=, -file2=, -file3=, etc.
        if (lower.find("-file") == 0) {
            size_t eq = p.find('=');
            if (eq != std::string::npos) {
                std::string filepath = p.substr(eq + 1);
                // Limpiar comillas si las hay
                if (!filepath.empty() && filepath.front() == '"')
                    filepath.erase(0, 1);
                if (!filepath.empty() && filepath.back() == '"')
                    filepath.pop_back();
                files.push_back(filepath);
            }
        }
    }

    if (files.empty()) {
        std::cout << "ERROR: Debe especificar al menos -file1=<ruta>\n";
        continue;
    }

    CatCmd cat;
    cat.execute(files);
}
// ================== REP ==================
else if (command == "rep") {
    std::string name = "", path = "", id = "", pathFileLs = "";

    for (int i = 1; i < (int)args.size(); i++) {
        std::string p = args[i];
        std::string lower = p;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        if      (lower.find("-name=") == 0)          name       = lower.substr(6);
        else if (lower.find("-path_file_ls=") == 0)  pathFileLs = p.substr(14);
        else if (lower.find("-path=") == 0)          path       = p.substr(6);
        else if (lower.find("-id=") == 0)            id         = p.substr(4);
    }

    if (name.empty() || path.empty() || id.empty()) {
        std::cout << "ERROR: -name, -path e -id son obligatorios\n";
        continue;
    }

    RepCmd rep;
    rep.execute(name, path, id, pathFileLs);
}
        else {
            std::cout << "Comando no reconocido\n";
        }
    }

    return 0;
}
