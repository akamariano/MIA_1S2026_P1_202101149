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
#include "../commands/remove_cmd.h"
#include "../commands/rename_cmd.h"
#include "../commands/copy_cmd.h"
#include "../commands/move_cmd.h"
#include "../commands/find_cmd.h"
#include "../commands/chown_cmd.h"
#include "../commands/chmod_cmd.h"
#include "../commands/journaling_cmd.h"
#include "../commands/loss_cmd.h"
#include <algorithm>

string captureOutput(function<void()> fn) {
    streambuf* oldBuf = cout.rdbuf();
    ostringstream ss;
    cout.rdbuf(ss.rdbuf());
    fn();
    cout.rdbuf(oldBuf);
    return ss.str();
}

// Limpia comillas al inicio y fin de un string
static string stripQuotes(const string& s) {
    string r = s;
    if (!r.empty() && r.front() == '"') r.erase(0, 1);
    if (!r.empty() && r.back()  == '"') r.pop_back();
    return r;
}

// Divide respetando comillas: -path="/home/mi carpeta" se trata como un token
vector<string> splitArgs(const string& input) {
    vector<string> tokens;
    string current;
    bool inQuotes = false;

    for (size_t i = 0; i < input.size(); i++) {
        char c = input[i];
        if (c == '"') {
            // Si estamos fuera de comillas y el siguiente char es espacio o fin -> comilla suelta, ignorar
            if (!inQuotes && (i+1 >= input.size() || input[i+1] == ' ')) {
                // comilla suelta al final de token, ignorar
                continue;
            }
            inQuotes = !inQuotes;
            // No agregar la comilla al token
        } else if (c == ' ' && !inQuotes) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if (!current.empty()) tokens.push_back(current);
    return tokens;
}

string processCommand(const string& rawInput) {
    if (rawInput.empty()) return "";

    // Limpiar comentarios inline (# fuera de comillas)
    string input;
    bool inQ = false;
    for (size_t i = 0; i < rawInput.size(); i++) {
        if (rawInput[i] == '"') inQ = !inQ;
        if (rawInput[i] == '#' && !inQ) break;
        input += rawInput[i];
    }
    // Trim
    while (!input.empty() && input.back() == ' ') input.pop_back();
    if (input.empty()) return "";

    vector<string> args = splitArgs(input);
    if (args.empty()) return "";

    string command = args[0];
    transform(command.begin(), command.end(), command.begin(), ::tolower);

    return captureOutput([&]() {

        // ================== MKDISK ==================
        if (command == "mkdisk") {
            int size = -1; char unit = 'M';
            string fit = "FF", path = "";
            bool hasUnknown = false;
            for (int i = 1; i < (int)args.size(); i++) {
                string p = args[i], lower = p;
                transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if      (lower.find("-size=") == 0) size = stoi(p.substr(6));
                else if (lower.find("-unit=") == 0) unit = toupper(p.substr(6)[0]);
                else if (lower.find("-fit=")  == 0) { fit = p.substr(5); transform(fit.begin(), fit.end(), fit.begin(), ::toupper); }
                else if (lower.find("-path=") == 0) path = stripQuotes(p.substr(6));
                else hasUnknown = true;
            }
            if (hasUnknown) { cout << "ERROR: Parámetros inválidos\n"; return; }
            if (size <= 0 || path.empty()) { cout << "ERROR: Parámetros inválidos\n"; return; }
            MkDisk mk; mk.execute(size, unit, fit, path);
        }

        // ================== RMDISK ==================
        else if (command == "rmdisk") {
            string path = "";
            for (int i = 1; i < (int)args.size(); i++) {
                string lower = args[i];
                transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower.find("-path=") == 0) path = stripQuotes(args[i].substr(6));
            }
            if (path.empty()) { cout << "ERROR: -path es obligatorio\n"; return; }
            RmDisk rm; rm.execute(path);
        }

        // ================== FDISK ==================
        else if (command == "fdisk") {
            int size = -1; char unit = 'B', type = 'P';
            string fit = "WF", path = "", name = "";
            for (int i = 1; i < (int)args.size(); i++) {
                string p = args[i], lower = p;
                transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if      (lower.find("-size=") == 0) size = stoi(p.substr(6));
                else if (lower.find("-unit=") == 0) unit = toupper(p.substr(6)[0]);
                else if (lower.find("-path=") == 0) path = stripQuotes(p.substr(6));
                else if (lower.find("-type=") == 0) type = toupper(p.substr(6)[0]);
                else if (lower.find("-fit=")  == 0) { fit = p.substr(5); transform(fit.begin(), fit.end(), fit.begin(), ::toupper); }
                else if (lower.find("-name=") == 0) name = stripQuotes(p.substr(6));
            }
            if (size <= 0 || path.empty() || name.empty()) { cout << "ERROR: Parámetros inválidos\n"; return; }
            FDisk fd; fd.execute(size, unit, path, type, fit, name);
        }

        // ================== MOUNT / MOUNTED ==================
        else if (command == "mount" || command == "mounted") {
            if (command == "mounted" || args.size() == 1) {
                MountManager::showMounted(); return;
            }
            string path = "", name = "";
            for (int i = 1; i < (int)args.size(); i++) {
                string p = args[i], lower = p;
                transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower.find("-path=") == 0) path = stripQuotes(p.substr(6));
                else if (lower.find("-name=") == 0) name = stripQuotes(p.substr(6));
            }
            if (path.empty() || name.empty()) { cout << "ERROR: -path y -name son obligatorios\n"; return; }
            Mount m; m.execute(path, name);
        }

        // ================== MKFS ==================
        else if (command == "mkfs") {
            string id = "", type = "full";
            for (int i = 1; i < (int)args.size(); i++) {
                string p = args[i], lower = p;
                transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower.find("-id=")   == 0) id   = p.substr(4);
                else if (lower.find("-type=") == 0) type = lower.substr(6);
            }
            if (id.empty()) { cout << "ERROR: -id es obligatorio\n"; return; }
            if (type != "full" && type != "ext3") {
                cout << "ERROR: -type debe ser 'full' (EXT2) o 'ext3'\n"; return;
            }
            Mkfs mkfs; mkfs.execute(id, type);
        }

        // ================== LOGIN ==================
        else if (command == "login") {
            string user = "", pass = "", id = "";
            for (int i = 1; i < (int)args.size(); i++) {
                string p = args[i], lower = p;
                transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if      (lower.find("-user=") == 0) user = stripQuotes(p.substr(6));
                else if (lower.find("-pass=") == 0) pass = stripQuotes(p.substr(6));
                else if (lower.find("-id=")   == 0) id   = p.substr(4);
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
                if (lower.find("-name=") == 0) name = stripQuotes(args[i].substr(6));
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
                if (lower.find("-name=") == 0) name = stripQuotes(args[i].substr(6));
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
                if      (lower.find("-user=") == 0) user = stripQuotes(p.substr(6));
                else if (lower.find("-pass=") == 0) pass = stripQuotes(p.substr(6));
                else if (lower.find("-grp=")  == 0) grp  = stripQuotes(p.substr(5));
            }
            if (user.empty() || pass.empty() || grp.empty()) {
                cout << "ERROR: -user, -pass y -grp son obligatorios\n"; return;
            }
            MkUsr mu; mu.execute(user, pass, grp);
        }

        // ================== RMUSR ==================
        else if (command == "rmusr") {
            string user = "";
            for (int i = 1; i < (int)args.size(); i++) {
                string lower = args[i];
                transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower.find("-user=") == 0) user = stripQuotes(args[i].substr(6));
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
                if      (lower.find("-user=") == 0) user = stripQuotes(p.substr(6));
                else if (lower.find("-grp=")  == 0) grp  = stripQuotes(p.substr(5));
            }
            if (user.empty() || grp.empty()) { cout << "ERROR: Parámetros inválidos\n"; return; }
            ChGrp cg; cg.execute(user, grp);
        }

        // ================== MKDIR ==================
        else if (command == "mkdir") {
            string path = ""; bool createParents = false;
            for (int i = 1; i < (int)args.size(); i++) {
                string p = args[i], lower = p;
                transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower == "-p") createParents = true;
                else if (lower.find("-path=") == 0) path = stripQuotes(p.substr(6));
            }
            if (path.empty()) { cout << "ERROR: -path es obligatorio\n"; return; }
            // Validar que el path sea válido (no solo backslash)
            if (path == "\\" || path == "/") {
                MkdirCmd md; md.execute(path, createParents);
            } else {
                MkdirCmd md; md.execute(path, createParents);
            }
        }

        // ================== MKFILE ==================
        else if (command == "mkfile") {
            string path = "", cont = ""; int size = 0; bool r = false;
            bool sizeNeg = false;
            for (int i = 1; i < (int)args.size(); i++) {
                string p = args[i], lower = p;
                transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower == "-r") r = true;
                else if (lower.find("-path=") == 0) path = stripQuotes(p.substr(6));
                else if (lower.find("-size=") == 0) {
                    int s = stoi(p.substr(6));
                    if (s < 0) { sizeNeg = true; }
                    else size = s;
                }
                else if (lower.find("-cont=") == 0) cont = stripQuotes(p.substr(6));
            }
            if (sizeNeg) { cout << "ERROR: -size no puede ser negativo\n"; return; }
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
                        string fp = stripQuotes(p.substr(eq + 1));
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
                // Limpiar comillas sueltas en cualquier posición
                string val = p;
                // Remover comillas de apertura o cierre sueltas
                while (!val.empty() && val.back() == '"') val.pop_back();
                while (!val.empty() && val.front() == '"') val.erase(0,1);
                string lval = lower;
                while (!lval.empty() && lval.back() == '"') lval.pop_back();
                while (!lval.empty() && lval.front() == '"') lval.erase(0,1);

                if      (lval.find("-name=") == 0)         name       = lval.substr(6);
                else if (lval.find("-path_file_ls=") == 0) pathFileLs = val.substr(14);
                else if (lval.find("-path=") == 0)         path       = val.substr(6);
                else if (lval.find("-id=") == 0)           id         = val.substr(4);
            }
            if (name.empty() || path.empty() || id.empty()) {
                cout << "ERROR: -name, -path e -id son obligatorios\n"; return;
            }
            RepCmd rep; rep.execute(name, path, id, pathFileLs);
        }
                // ================== REMOVE ==================
        else if (command == "remove") {
            string path = "";
            for (int i = 1; i < (int)args.size(); i++) {
                string p = args[i], lower = p;
                transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower.find("-path=") == 0) path = stripQuotes(p.substr(6));
            }
            if (path.empty()) { cout << "ERROR: -path es obligatorio\n"; return; }
            RemoveCmd rm; rm.execute(path);
        }
        // RENAME
else if (command == "rename") {
    string path = "", name = "";
    for (int i = 1; i < (int)args.size(); i++) {
        string p = args[i], lower = p;
        transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        if (lower.find("-path=") == 0) path = stripQuotes(p.substr(6));
        else if (lower.find("-name=") == 0) name = stripQuotes(p.substr(6));
    }
    if (path.empty() || name.empty()) {
        cout << "ERROR: -path y -name son obligatorios\n"; return;
    }
    RenameCmd rc; rc.execute(path, name);
}
        //COPY
        else if (command == "copy") {
            string path = "", destino = "";
            for (int i = 1; i < (int)args.size(); i++) {
                string p = args[i], lower = p;
                transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower.find("-path=") == 0)    path    = stripQuotes(p.substr(6));
                else if (lower.find("-destino=") == 0) destino = stripQuotes(p.substr(9));
            }
            if (path.empty() || destino.empty()) {
                cout << "ERROR: -path y -destino son obligatorios\n"; return;
            }
            CopyCmd cc; cc.execute(path, destino);
        }
        //MOVE
                else if (command == "move") {
            string path = "", destino = "";
            for (int i = 1; i < (int)args.size(); i++) {
                string p = args[i], lower = p;
                transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower.find("-path=") == 0)         path    = stripQuotes(p.substr(6));
                else if (lower.find("-destino=") == 0) destino = stripQuotes(p.substr(9));
            }
            if (path.empty() || destino.empty()) {
                cout << "ERROR: -path y -destino son obligatorios\n"; return;
            }
            MoveCmd mc; mc.execute(path, destino);
        }
        //FIND
        else if (command == "find") {
    string path = "", name = "";
    for (int i = 1; i < (int)args.size(); i++) {
        string p = args[i], lower = p;
        transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        if (lower.find("-path=") == 0) path = stripQuotes(p.substr(6));
        else if (lower.find("-name=") == 0) name = stripQuotes(p.substr(6));
    }
    if (path.empty() || name.empty()) {
        cout << "ERROR: -path y -name son obligatorios\n"; return;
    }
    FindCmd fc; fc.execute(path, name);
}
// CHOWN
else if (command == "chown") {
    string path = "", usuario = ""; bool r = false;
    for (int i = 1; i < (int)args.size(); i++) {
        string p = args[i], lower = p;
        transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        if (lower == "-r") r = true;
        else if (lower.find("-path=") == 0)    path    = stripQuotes(p.substr(6));
        else if (lower.find("-usuario=") == 0) usuario = stripQuotes(p.substr(9));
    }
    if (path.empty() || usuario.empty()) {
        cout << "ERROR: -path y -usuario son obligatorios\n"; return;
    }
    ChownCmd cc; cc.execute(path, usuario, r);
}

// CHMOD
else if (command == "chmod") {
    string path = "", ugo = ""; bool r = false;
    for (int i = 1; i < (int)args.size(); i++) {
        string p = args[i], lower = p;
        transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        if (lower == "-r") r = true;
        else if (lower.find("-path=") == 0) path = stripQuotes(p.substr(6));
        else if (lower.find("-ugo=") == 0)  ugo  = p.substr(5);
    }
    if (path.empty() || ugo.empty()) {
        cout << "ERROR: -path y -ugo son obligatorios\n"; return;
    }
    ChmodCmd cm; cm.execute(path, ugo, r);
}
// LOSS
else if (command == "loss") {
    string id = "";
    for (int i = 1; i < (int)args.size(); i++) {
        string p = args[i], lower = p;
        transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        if (lower.find("-id=") == 0) id = p.substr(4);
    }
    if (id.empty()) { cout << "ERROR: -id es obligatorio\n"; return; }
    LossCmd lc; lc.execute(id);
}

// JOURNALING
else if (command == "journaling") {
    string id = "";
    for (int i = 1; i < (int)args.size(); i++) {
        string p = args[i], lower = p;
        transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        if (lower.find("-id=") == 0) id = p.substr(4);
    }
    if (id.empty()) { cout << "ERROR: -id es obligatorio\n"; return; }
    JournalingCmd jc; jc.execute(id);
}
        else {
            cout << "ERROR: Comando '" << command << "' no reconocido\n";
        }
    });
}

void startServer(int port) {
    Server svr;

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

    svr.Get("/status", [](const Request&, Response& res) {
        res.set_content("{\"status\":\"ok\"}", "application/json");
    });

    svr.Post("/command", [](const Request& req, Response& res) {
        string body = req.body;
        string command = "";
        // Extraer valor de "command" respetando \" escapadas
        
        size_t pos = body.find("\"command\"");
        if (pos != string::npos) {
            size_t start = body.find("\"", pos + 9);
            if (start != string::npos) {
                start++; // saltar la comilla de apertura
                string raw;
                bool escaped = false;
                for (size_t i = start; i < body.size(); i++) {
                    if (escaped) {
                        raw += body[i];
                        escaped = false;
                    } else if (body[i] == '\\') {
                        raw += body[i];
                        escaped = true;
                    } else if (body[i] == '"') {
                        break; // comilla de cierre real
                    } else {
                        raw += body[i];
                    }
                }
                command = raw;
            }
        }
        if (command.empty()) {
            res.status = 400;
            res.set_content("{\"error\":\"Comando vacío\"}", "application/json");
            return;
        }

       // Desencode escapes del JSON: \n -> newline, \" -> ", \\ -> backslash
        string decoded;
        for (size_t i = 0; i < command.size(); i++) {
            if (command[i] == '\\' && i+1 < command.size()) {
                char next = command[i+1];
                if      (next == 'n')  { decoded += '\n'; i++; }
                else if (next == '"')  { decoded += '"';  i++; }
                else if (next == '\\') { decoded += '\\'; i++; }
                else if (next == 't')  { decoded += '\t'; i++; }
                else if (next == 'r')  { decoded += '\r'; i++; }
                else { decoded += command[i]; }
            } else {
                decoded += command[i];
            }
        }

        string fullOutput = "";
        istringstream ss(decoded);
        string line;
        while (getline(ss, line)) {
            // Trim
            while (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty() || line[0] == '#') continue;
            string out = processCommand(line);
            if (!out.empty()) fullOutput += out;
        }

        string escaped = "";
        for (char c : fullOutput) {
            if      (c == '"')  escaped += "\\\"";
            else if (c == '\\') escaped += "\\\\";
            else if (c == '\n') escaped += "\\n";
            else if (c == '\r') escaped += "\\r";
            else escaped += c;
        }

        res.set_content("{\"output\":\"" + escaped + "\"}", "application/json");
    });

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
        string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        res.set_content(content, contentType.c_str());
    });

    cout << "==============================\n";
    cout << " EXTREAMFS SERVER\n";
    cout << " Puerto: " << port << "\n";
    cout << " http://localhost:" << port << "\n";
    cout << "==============================\n";

    svr.listen("0.0.0.0", port);
}