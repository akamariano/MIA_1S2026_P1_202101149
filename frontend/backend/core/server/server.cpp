#include "server.h"
#include "../../include/httplib.h"
#include "../mount/mount_manager.h"
#include <iostream>
#include <sstream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <fstream>
#include <filesystem>

using namespace std;
using namespace httplib;

// Incluir todas las clases de comandos que se pueden ejecutar
// El servidor reutiliza el parsing y ejecución del backend
#include "../commands/mkdisk.h"
#include "../commands/fdisk.h"
#include "../commands/rmdisk.h"
#include "../commands/mount.h"
#include "../commands/mkfs.h"
#include "../commands/mkgrp.h"
#include "../commands/rmgrp.h"
#include "../commands/mkusr.h"
#include "../commands/rmusr.h"
#include "../commands/chgrp.h"
#include "../commands/mkdir_cmd.h"
#include "../commands/mkfile_cmd.h"
#include "../commands/cat_cmd.h"
#include "../commands/rep_cmd.h"
#include "../filesystem/login.h"
#include "../filesystem/logout.h"
#include "../filesystem/session_manager.h"

#include <algorithm>

// Capturar stdout redirigiendo el buffer de cout hacia un stringstream
string captureOutput(function<void()> fn) {
    // Guardar el buffer original
    streambuf* oldBuf = cout.rdbuf();
    ostringstream ss;
    cout.rdbuf(ss.rdbuf());
    
    fn();
    
    // Restaurar el buffer original
    cout.rdbuf(oldBuf);
    return ss.str();
}

// Dividir un comando en argumentos separados por espacios
vector<string> splitArgs(const string& input) {
    vector<string> tokens;
    istringstream ss(input);
    string word;
    while (ss >> word) tokens.push_back(word);
    return tokens;
}

// Procesar un comando y retornar su salida
string processCommand(const string& input) {
    if (input.empty()) return "";

    vector<string> args = splitArgs(input);
    if (args.empty()) return "";

    string command = args[0];
    transform(command.begin(), command.end(), command.begin(), ::tolower);

    return captureOutput([&]() {

        // Ejecutar comando mkdisk
        if (command == "mkdisk") {
            int size = -1; char unit = 'M';
            string fit = "FF", path = "";
            for (int i = 1; i < (int)args.size(); i++) {
                string p = args[i], lower = p;
                transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower.find("-size=") == 0) size = stoi(p.substr(6));
                else if (lower.find("-unit=") == 0) unit = toupper(p.substr(6)[0]);
                else if (lower.find("-fit=") == 0) { fit = p.substr(5); transform(fit.begin(), fit.end(), fit.begin(), ::toupper); }
                else if (lower.find("-path=") == 0) path = p.substr(6);
            }
            if (size <= 0 || path.empty()) { cout << "ERROR: Parámetros inválidos\n"; return; }
            MkDisk mk; mk.execute(size, unit, fit, path);
        }

        // ================== FDISK ==================
        else if (command == "fdisk") {
            int size = -1; char unit = 'K', type = 'P';
            string fit = "WF", path = "", name = "";
            for (int i = 1; i < (int)args.size(); i++) {
                string p = args[i], lower = p;
                transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower.find("-size=") == 0) size = stoi(p.substr(6));
                else if (lower.find("-unit=") == 0) unit = toupper(p.substr(6)[0]);
                else if (lower.find("-path=") == 0) path = p.substr(6);
                else if (lower.find("-type=") == 0) type = toupper(p.substr(6)[0]);
                else if (lower.find("-fit=") == 0) { fit = p.substr(5); transform(fit.begin(), fit.end(), fit.begin(), ::toupper); }
                else if (lower.find("-name=") == 0) name = p.substr(6);
            }
            if (size <= 0 || path.empty() || name.empty()) { cout << "ERROR: Parámetros inválidos\n"; return; }
            FDisk fd; fd.execute(size, unit, path, type, fit, name);
        }

        // ================== RMDISK ==================
        else if (command == "rmdisk") {
            string path = "";
            for (int i = 1; i < (int)args.size(); i++) {
                string lower = args[i];
                transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower.find("-path=") == 0) path = args[i].substr(6);
            }
            if (path.empty()) { cout << "ERROR: -path es obligatorio\n"; return; }
            RmDisk rm; rm.execute(path);
        }

        // ================== MOUNT ==================
        else if (command == "mount") {
            if (args.size() == 1) { MountManager::showMounted(); return; }
            string path = "", name = "";
            for (int i = 1; i < (int)args.size(); i++) {
                if (args[i].find("-path=") == 0) path = args[i].substr(6);
                else if (args[i].find("-name=") == 0) name = args[i].substr(6);
            }
            if (path.empty() || name.empty()) { cout << "ERROR: -path y -name son obligatorios\n"; return; }
            Mount m; m.execute(path, name);
        }

        // ================== MKFS ==================
        else if (command == "mkfs") {
            string id = "", type = "full";
            for (int i = 1; i < (int)args.size(); i++) {
                string lower = args[i];
                transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower.find("-id=") == 0) id = args[i].substr(4);
                else if (lower.find("-type=") == 0) type = lower.substr(6);
            }
            if (id.empty()) { cout << "ERROR: -id es obligatorio\n"; return; }
            Mkfs mkfs; mkfs.execute(id);
        }

        // ================== LOGIN ==================
        else if (command == "login") {
            string user = "", pass = "", id = "";
            for (int i = 1; i < (int)args.size(); i++) {
                string p = args[i], lower = p;
                transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower.find("-user=") == 0) user = p.substr(6);
                else if (lower.find("-pass=") == 0) pass = p.substr(6);
                else if (lower.find("-id=") == 0) id = p.substr(4);
            }
            if (user.empty() || pass.empty() || id.empty()) { cout << "ERROR: Parámetros inválidos\n"; return; }
            Login l; l.execute(user, pass, id);
        }

        // ================== LOGOUT ==================
        else if (command == "logout") {
            Logout lo; lo.execute();
        }

        // ================== MKGRP ==================
        else if (command == "mkgrp") {
            string name = "";
            for (int i = 1; i < (int)args.size(); i++) {
                string lower = args[i];
                transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower.find("-name=") == 0) name = args[i].substr(6);
            }
            if (name.empty()) { cout << "ERROR: -name es obligatorio\n"; return; }
            MkGrp mg; mg.execute(name);
        }

        // ================== RMGRP ==================
        else if (command == "rmgrp") {
            string name = "";
            for (int i = 1; i < (int)args.size(); i++) {
                string lower = args[i];
                transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower.find("-name=") == 0) name = args[i].substr(6);
            }
            if (name.empty()) { cout << "ERROR: -name es obligatorio\n"; return; }
            RmGrp rg; rg.execute(name);
        }

        // ================== MKUSR ==================
        else if (command == "mkusr") {
            string user = "", pass = "", grp = "";
            for (int i = 1; i < (int)args.size(); i++) {
                string p = args[i], lower = p;
                transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower.find("-user=") == 0) user = p.substr(6);
                else if (lower.find("-pass=") == 0) pass = p.substr(6);
                else if (lower.find("-grp=") == 0) grp = p.substr(5);
            }
            if (user.empty() || pass.empty() || grp.empty()) { cout << "ERROR: Parámetros inválidos\n"; return; }
            MkUsr mu; mu.execute(user, pass, grp);
        }

        // ================== RMUSR ==================
        else if (command == "rmusr") {
            string user = "";
            for (int i = 1; i < (int)args.size(); i++) {
                string lower = args[i];
                transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower.find("-user=") == 0) user = args[i].substr(6);
            }
            if (user.empty()) { cout << "ERROR: -user es obligatorio\n"; return; }
            RmUsr ru; ru.execute(user);
        }

        // ================== CHGRP ==================
        else if (command == "chgrp") {
            string user = "", grp = "";
            for (int i = 1; i < (int)args.size(); i++) {
                string p = args[i], lower = p;
                transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower.find("-user=") == 0) user = p.substr(6);
                else if (lower.find("-grp=") == 0) grp = p.substr(5);
            }
            if (user.empty() || grp.empty()) { cout << "ERROR: Parámetros inválidos\n"; return; }
            ChGrp cg; cg.execute(user, grp);
        }

        // ================== MKDIR ==================
        else if (command == "mkdir") {
            string path = ""; bool createParents = false;
            for (int i = 1; i < (int)args.size(); i++) {
                string lower = args[i];
                transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower == "-p") createParents = true;
                else if (lower.find("-path=") == 0) path = args[i].substr(6);
            }
            if (path.empty()) { cout << "ERROR: -path es obligatorio\n"; return; }
            MkdirCmd md; md.execute(path, createParents);
        }

        // ================== MKFILE ==================
        else if (command == "mkfile") {
            string path = "", cont = ""; int size = 0; bool r = false;
            for (int i = 1; i < (int)args.size(); i++) {
                string p = args[i], lower = p;
                transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower == "-r") r = true;
                else if (lower.find("-path=") == 0) path = p.substr(6);
                else if (lower.find("-size=") == 0) size = stoi(p.substr(6));
                else if (lower.find("-cont=") == 0) cont = p.substr(6);
            }
            if (path.empty()) { cout << "ERROR: -path es obligatorio\n"; return; }
            MkfileCmd mf; mf.execute(path, r, size, cont);
        }

        // ================== CAT ==================
        else if (command == "cat") {
            vector<string> files;
            for (int i = 1; i < (int)args.size(); i++) {
                string p = args[i], lower = p;
                transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower.find("-file") == 0) {
                    size_t eq = p.find('=');
                    if (eq != string::npos) {
                        string fp = p.substr(eq + 1);
                        if (!fp.empty() && fp.front() == '"') fp.erase(0,1);
                        if (!fp.empty() && fp.back() == '"') fp.pop_back();
                        files.push_back(fp);
                    }
                }
            }
            if (files.empty()) { cout << "ERROR: Debe especificar al menos -file1\n"; return; }
            CatCmd cat; cat.execute(files);
        }

        // ================== REP ==================
        else if (command == "rep") {
            string name = "", path = "", id = "", pathFileLs = "";
            for (int i = 1; i < (int)args.size(); i++) {
                string p = args[i], lower = p;
                transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower.find("-name=") == 0) name = lower.substr(6);
                else if (lower.find("-path_file_ls=") == 0) pathFileLs = p.substr(14);
                else if (lower.find("-path=") == 0) path = p.substr(6);
                else if (lower.find("-id=") == 0) id = p.substr(4);
            }
            if (name.empty() || path.empty() || id.empty()) { cout << "ERROR: Parámetros inválidos\n"; return; }
            RepCmd rep; rep.execute(name, path, id, pathFileLs);
        }

        else {
            cout << "ERROR: Comando '" << command << "' no reconocido\n";
        }
    });
}

void startServer(int port) {
    Server svr;

    // ===== CORS =====
    svr.set_pre_routing_handler([](const Request& req, Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        if (req.method == "OPTIONS") {
            res.status = 204;
            return Server::HandlerResponse::Handled;
        }
        return Server::HandlerResponse::Unhandled;
    });

    // ===== GET /status =====
    svr.Get("/status", [](const Request&, Response& res) {
        res.set_content("{\"status\":\"ok\"}", "application/json");
    });

    // ===== POST /command =====
    svr.Post("/command", [](const Request& req, Response& res) {
        // Parsear JSON simple
        string body = req.body;
        string command = "";

        // Buscar "command":"..."
        size_t pos = body.find("\"command\"");
        if (pos != string::npos) {
            size_t start = body.find("\"", pos + 9);
            if (start != string::npos) {
                start++;
                size_t end = body.find("\"", start);
                if (end != string::npos) {
                    command = body.substr(start, end - start);
                }
            }
        }

        if (command.empty()) {
            res.status = 400;
            res.set_content("{\"error\":\"Comando vacío\"}", "application/json");
            return;
        }

        // Procesar línea por línea si hay múltiples comandos
        string fullOutput = "";
        istringstream ss(command);
        string line;
        while (getline(ss, line)) {
            if (line.empty() || line[0] == '#') continue;
            string out = processCommand(line);
            if (!out.empty()) fullOutput += out;
        }

        // Escapar output para JSON
        string escaped = "";
        for (char c : fullOutput) {
            if (c == '"') escaped += "\\\"";
            else if (c == '\\') escaped += "\\\\";
            else if (c == '\n') escaped += "\\n";
            else if (c == '\r') escaped += "\\r";
            else escaped += c;
        }

        string json = "{\"output\":\"" + escaped + "\"}";
        res.set_content(json, "application/json");
    });

    // ===== GET /report?path=/ruta/archivo.jpg =====
    svr.Get("/report", [](const Request& req, Response& res) {
        if (!req.has_param("path")) {
            res.status = 400;
            res.set_content("{\"error\":\"path requerido\"}", "application/json");
            return;
        }

        string filePath = req.get_param_value("path");

        ifstream file(filePath, ios::binary);
        if (!file.is_open()) {
            res.status = 404;
            res.set_content("{\"error\":\"Archivo no encontrado\"}", "application/json");
            return;
        }

        string ext = filesystem::path(filePath).extension().string();
        string contentType = "application/octet-stream";
        if (ext == ".jpg" || ext == ".jpeg") contentType = "image/jpeg";
        else if (ext == ".png") contentType = "image/png";
        else if (ext == ".txt") contentType = "text/plain";
        else if (ext == ".pdf") contentType = "application/pdf";

        string content((istreambuf_iterator<char>(file)),
                        istreambuf_iterator<char>());
        res.set_content(content, contentType.c_str());
    });

    cout << "==============================\n";
    cout << " EXTREAMFS SERVER\n";
    cout << " Puerto: " << port << "\n";
    cout << " http://localhost:" << port << "\n";
    cout << "==============================\n";

    svr.listen("0.0.0.0", port);
}