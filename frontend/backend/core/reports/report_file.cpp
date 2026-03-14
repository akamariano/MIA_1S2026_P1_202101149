#include "report_file.h"
#include "report_utils.h"
#include "../core/filesystem/EXT2Utils.h"
#include "../core/filesystem/SuperBlock.h"
#include "../core/filesystem/Inode.h"
#include "../core/filesystem/Blocks.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <algorithm>
using namespace std;

void ReportFile::generate(FILE* disk, MountedPartition* part,
                           const string& outPath, const string& filePath,
                           const string& extension) {
    if (filePath.empty()) {
        cout << "ERROR: -path_file_ls es obligatorio para reporte file\n";
        return;
    }

    SuperBlock sb;
    readSuperBlock(disk, part->start, sb);

    int inodeNum = resolvePath(disk, sb, filePath);
    if (inodeNum == -1) {
        cout << "ERROR: El archivo '" << filePath << "' no existe\n";
        return;
    }

    Inode inode;
    readInode(disk, sb, inodeNum, inode);

    if (inode.i_type != '1') {
        cout << "ERROR: '" << filePath << "' no es un archivo\n";
        return;
    }

    // Leer contenido del archivo (bloques directos)
    string fileContent;
    for (int b = 0; b < 12; b++) {
        if (inode.i_block[b] == -1) break;
        FileBlock fb;
        memset(&fb, 0, sizeof(FileBlock));
        readBlock(disk, sb, inode.i_block[b], &fb);
        int toRead = min((int)sizeof(fb.b_content),
                         inode.i_size - (int)fileContent.size());
        if (toRead <= 0) break;
        fileContent.append(fb.b_content, toRead);
    }

    // Detectar extensión del outPath
    string ext = extension;
    if (ext.empty()) {
        size_t dot = outPath.rfind('.');
        if (dot != string::npos) {
            ext = outPath.substr(dot);
            transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        }
    }

    if (ext == ".txt") {
        // Generar archivo de texto plano con solo el contenido
        writeTxt(fileContent, outPath);
    } else {
        // Renderizar como imagen con Graphviz (.jpg, .png, etc.)
        string escaped;
        for (char c : fileContent) {
            if      (c == '<')  escaped += "&lt;";
            else if (c == '>')  escaped += "&gt;";
            else if (c == '&')  escaped += "&amp;";
            else if (c == '"')  escaped += "&quot;";
            else if (c == '\n') escaped += "<BR/>";
            else                escaped += c;
        }

        ostringstream dot;
        dot << "digraph FILE {\n";
        dot << "  node [shape=plaintext fontname=\"Courier\"]\n\n";
        dot << "  file [label=<\n";
        dot << "    <TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"8\">\n";
        dot << "      <TR><TD BGCOLOR=\"#1A5276\" ALIGN=\"CENTER\">"
            << "<FONT COLOR=\"white\"><B>" << filePath << "</B></FONT>"
            << "</TD></TR>\n";
        dot << "      <TR><TD BGCOLOR=\"#FDFEFE\" ALIGN=\"LEFT\">"
            << "<FONT FACE=\"Courier\">" << escaped << "</FONT>"
            << "</TD></TR>\n";
        dot << "    </TABLE>>];\n";
        dot << "}\n";

        renderDot(dot.str(), outPath);
    }

    cout << "OK: Reporte generado en " << outPath << "\n";
}