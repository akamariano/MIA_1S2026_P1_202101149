#include "report_ls.h"
#include "report_utils.h"
#include "../core/filesystem/EXT2Utils.h"
#include "../core/filesystem/SuperBlock.h"
#include "../core/filesystem/Inode.h"
#include "../core/filesystem/Blocks.h"
#include <sstream>
#include <cstring>
#include <ctime>
#include <map>
using namespace std;

// Leer el archivo /users.txt y construir mapas que relacionan IDs con nombres
static void loadUsers(FILE* disk, const SuperBlock& sb,
                      map<int,string>& userMap, map<int,string>& groupMap) {
    // Encontrar el inode del archivo users.txt
    int usersInode = resolvePath(disk, sb, "/users.txt");
    if (usersInode == -1) return;

    // Leer el inode y su contenido
    Inode inode;
    readInode(disk, sb, usersInode, inode);

    // Leer todo el contenido del archivo
    string content;
    for (int b = 0; b < 12; b++) {
        if (inode.i_block[b] == -1) break;
        FileBlock fb;
        memset(&fb, 0, sizeof(FileBlock));
        readBlock(disk, sb, inode.i_block[b], &fb);
        for (int c = 0; c < 64; c++) {
            if (fb.b_content[c] == 0) break;
            content += fb.b_content[c];
        }
    }

    // Parsear el contenido línea por línea
    // Formato grupos: ID,G,nombre
    // Formato usuarios: ID,U,grupo,nombre,pass
    istringstream ss(content);
    string line;
    while (getline(ss, line)) {
        if (line.empty()) continue;
        istringstream ls(line);
        string id_s, type, f1, f2, f3;
        getline(ls, id_s, ',');
        getline(ls, type, ',');
        getline(ls, f1, ',');
        int id = stoi(id_s);
        if (id == 0) continue;
        if (type == "G") {
            groupMap[id] = f1;
        } else if (type == "U") {
            // Campo 1 es grupo, campo 2 es nombre del usuario
            getline(ls, f2, ',');
            userMap[id] = f2;
        }
    }
}

void ReportLs::generate(FILE* disk, MountedPartition* part,
                         const string& outPath, const string& dirPath) {
    if (dirPath.empty()) {
        cout << "ERROR: -path_file_ls es obligatorio para reporte ls\n";
        return;
    }

    SuperBlock sb;
    readSuperBlock(disk, part->start, sb);

    // Cargar mapas de usuarios y grupos desde users.txt
    map<int,string> userMap, groupMap;
    loadUsers(disk, sb, userMap, groupMap);

    // Encontrar el inode del directorio
    int inodeNum = resolvePath(disk, sb, dirPath);
    if (inodeNum == -1) {
        cout << "ERROR: La ruta '" << dirPath << "' no existe\n";
        return;
    }

    Inode dirInode;
    readInode(disk, sb, inodeNum, dirInode);

    if (dirInode.i_type != '0') {
        cout << "ERROR: '" << dirPath << "' no es una carpeta\n";
        return;
    }

    auto fmtDate = [](time_t t) -> pair<string,string> {
        if (t == 0) return {"N/A","N/A"};
        char db[16], tb[16];
        struct tm* ti = localtime(&t);
        strftime(db, sizeof(db), "%d/%m/%Y", ti);
        strftime(tb, sizeof(tb), "%H:%M", ti);
        return {string(db), string(tb)};
    };

    auto fmtPerm = [](int perm) -> string {
        string r;
        int d[3] = {(perm/100)%10, (perm/10)%10, perm%10};
        for (int x : d) {
            r += (x&4) ? "r" : "-";
            r += (x&2) ? "w" : "-";
            r += (x&1) ? "x" : "-";
        }
        return r;
    };

    ostringstream dot;
    dot << "digraph LS {\n";
    dot << "  node [shape=plaintext fontname=\"Helvetica\"]\n\n";
    dot << "  ls [label=<\n";
    dot << "    <TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"6\">\n";

    dot << "      <TR><TD COLSPAN=\"8\" BGCOLOR=\"#1A5276\" ALIGN=\"CENTER\">"
        << "<FONT COLOR=\"white\"><B>LS: " << dirPath << "</B></FONT></TD></TR>\n";
    dot << "      <TR>"
        << "<TD BGCOLOR=\"#2E86C1\"><FONT COLOR=\"white\"><B>Permisos</B></FONT></TD>"
        << "<TD BGCOLOR=\"#2E86C1\"><FONT COLOR=\"white\"><B>Owner</B></FONT></TD>"
        << "<TD BGCOLOR=\"#2E86C1\"><FONT COLOR=\"white\"><B>Grupo</B></FONT></TD>"
        << "<TD BGCOLOR=\"#2E86C1\"><FONT COLOR=\"white\"><B>Size (en Bytes)</B></FONT></TD>"
        << "<TD BGCOLOR=\"#2E86C1\"><FONT COLOR=\"white\"><B>Fecha</B></FONT></TD>"
        << "<TD BGCOLOR=\"#2E86C1\"><FONT COLOR=\"white\"><B>Hora</B></FONT></TD>"
        << "<TD BGCOLOR=\"#2E86C1\"><FONT COLOR=\"white\"><B>Tipo</B></FONT></TD>"
        << "<TD BGCOLOR=\"#2E86C1\"><FONT COLOR=\"white\"><B>Name</B></FONT></TD>"
        << "</TR>\n";

    for (int b = 0; b < 12; b++) {
        if (dirInode.i_block[b] == -1) break;
        DirectoryBlock db;
        memset(&db, 0, sizeof(DirectoryBlock));
        readBlock(disk, sb, dirInode.i_block[b], &db);

        // Iterar sobre las entradas del bloque de directorio
        for (int e = 0; e < 4; e++) {
            int einode = db.b_content[e].b_inodo;
            if (einode == -1) continue;

            string ename(db.b_content[e].b_name,
                         strnlen(db.b_content[e].b_name, 12));
            if (ename.empty()) continue;
            
            // No mostrar las entradas de directorio padre
            if (ename == "." || ename == "..") continue;

            Inode child;
            readInode(disk, sb, einode, child);

            string typeStr = (child.i_type == '0') ? "Carpeta" : "Archivo";
            string bgColor = (child.i_type == '0') ? "#D6EAF8" : "#D5F5E3";
            string typePrefix = (child.i_type == '0') ? "d" : "-";
            string permStr = typePrefix + fmtPerm(child.i_perm);//linux sheet implementado fix
            auto [fecha, hora] = fmtDate(child.i_ctime);

            // Obtener nombres reales del propietario y grupo usando los mapas
            string ownerName = "root";
            string groupName = "root";
            if (child.i_uid > 0 && userMap.count(child.i_uid))
                ownerName = userMap[child.i_uid];
            if (child.i_gid > 0 && groupMap.count(child.i_gid))
                groupName = groupMap[child.i_gid];

            // Añadir fila a la tabla del reporte
            dot << "      <TR>"
                << "<TD BGCOLOR=\"" << bgColor << "\">" << permStr << "</TD>"
                << "<TD BGCOLOR=\"" << bgColor << "\">" << ownerName << "</TD>"
                << "<TD BGCOLOR=\"" << bgColor << "\">" << groupName << "</TD>"
                << "<TD BGCOLOR=\"" << bgColor << "\">" << child.i_size << "</TD>"
                << "<TD BGCOLOR=\"" << bgColor << "\">" << fecha << "</TD>"
                << "<TD BGCOLOR=\"" << bgColor << "\">" << hora << "</TD>"
                << "<TD BGCOLOR=\"" << bgColor << "\">" << typeStr << "</TD>"
                << "<TD BGCOLOR=\"" << bgColor << "\">" << ename << "</TD>"
                << "</TR>\n";
        }
    }

    dot << "    </TABLE>>];\n";
    dot << "}\n";

    renderDot(dot.str(), outPath);
}
