#include "report_tree.h"
#include "report_utils.h"
#include "../core/filesystem/EXT2Utils.h"
#include "../core/filesystem/SuperBlock.h"
#include "../core/filesystem/Inode.h"
#include "../core/filesystem/Blocks.h"
#include <sstream>
#include <cstring>
#include <set>
using namespace std;

static set<int> visitedInodes;
static set<int> visitedBlocks;

static string escapeContent(const char* buf, int maxLen) {
    string content;
    for (int c = 0; c < maxLen; c++) {
        unsigned char ch = buf[c];
        if (ch == 0) break;
        if      (ch == '<')  content += "&lt;";
        else if (ch == '>')  content += "&gt;";
        else if (ch == '&')  content += "&amp;";
        else if (ch == '"')  content += "&quot;";
        else if (ch == '\n' || ch == '\r') content += " ";
        else if (ch >= 32 && ch < 127) content += (char)ch;
    }
    if (content.size() > 40) content = content.substr(0, 40) + "...";
    return content;
}

static void renderInode(FILE* disk, SuperBlock& sb, int inodeNum,
                        ostringstream& nodes, ostringstream& edges);

static void renderDirBlock(FILE* disk, SuperBlock& sb, int blockNum,
                           int parentInode,
                           ostringstream& nodes, ostringstream& edges) {
    if (visitedBlocks.count(blockNum)) return;
    visitedBlocks.insert(blockNum);

    DirectoryBlock db;
    memset(&db, 0, sizeof(DirectoryBlock));
    readBlock(disk, sb, blockNum, &db);

    string blockId = "block" + to_string(blockNum);

    // 1. Primero escribir el nodo completo del bloque
    nodes << "  " << blockId << " [label=<\n";
    nodes << "    <TABLE BORDER=\"1\" CELLBORDER=\"1\" CELLSPACING=\"0\">\n";
    nodes << "      <TR><TD COLSPAN=\"2\" BGCOLOR=\"#0277BD\">"
          << "<FONT COLOR=\"white\"><B>Bloque Dir " << blockNum << "</B></FONT></TD></TR>\n";
    nodes << "      <TR><TD BGCOLOR=\"#B3E5FC\"><B>Nombre</B></TD>"
          << "<TD BGCOLOR=\"#B3E5FC\"><B>Inodo</B></TD></TR>\n";

    for (int e = 0; e < 4; e++) {
        int einode = db.b_content[e].b_inodo;
        string ename(db.b_content[e].b_name, strnlen(db.b_content[e].b_name, 12));
        string einodeStr = (einode == -1) ? "-" : to_string(einode);
        nodes << "      <TR><TD>" << (ename.empty() ? "-" : ename)
              << "</TD><TD>" << einodeStr << "</TD></TR>\n";
    }
    nodes << "    </TABLE>>];\n\n";

    // 2. Luego procesar recursión y edges
    for (int e = 0; e < 4; e++) {
        int einode = db.b_content[e].b_inodo;
        string ename(db.b_content[e].b_name, strnlen(db.b_content[e].b_name, 12));

        if (einode >= 0 && einode != parentInode &&
            ename != "." && ename != "..") {
            edges << "  " << blockId << " -> inode" << einode << ";\n";
            renderInode(disk, sb, einode, nodes, edges);
        }
    }
}

static void renderFileBlock(FILE* disk, SuperBlock& sb, int blockNum,
                             ostringstream& nodes) {
    if (visitedBlocks.count(blockNum)) return;
    visitedBlocks.insert(blockNum);

    FileBlock fb;
    memset(&fb, 0, sizeof(FileBlock));
    readBlock(disk, sb, blockNum, &fb);

    string content = escapeContent(fb.b_content, 64);
    string blockId = "block" + to_string(blockNum);

    nodes << "  " << blockId << " [label=<\n";
    nodes << "    <TABLE BORDER=\"1\" CELLBORDER=\"1\" CELLSPACING=\"0\">\n";
    nodes << "      <TR><TD BGCOLOR=\"#388E3C\">"
          << "<FONT COLOR=\"white\"><B>Bloque Arch " << blockNum << "</B></FONT></TD></TR>\n";
    nodes << "      <TR><TD BGCOLOR=\"#E8F5E9\">" << content << "</TD></TR>\n";
    nodes << "    </TABLE>>];\n\n";
}

static void renderInode(FILE* disk, SuperBlock& sb, int inodeNum,
                        ostringstream& nodes, ostringstream& edges) {
    if (visitedInodes.count(inodeNum)) return;
    visitedInodes.insert(inodeNum);

    Inode inode;
    readInode(disk, sb, inodeNum, inode);

    string nodeId  = "inode" + to_string(inodeNum);
    string typeStr = (inode.i_type == '0') ? "Carpeta" : "Archivo";
    string bgColor = (inode.i_type == '0') ? "#1565C0" : "#2E7D32";
    string bgLight = (inode.i_type == '0') ? "#E3F2FD" : "#E8F5E9";

    // Escribir nodo inodo completo
    nodes << "  " << nodeId << " [label=<\n";
    nodes << "    <TABLE BORDER=\"1\" CELLBORDER=\"1\" CELLSPACING=\"0\">\n";
    nodes << "      <TR><TD COLSPAN=\"2\" BGCOLOR=\"" << bgColor
          << "\"><FONT COLOR=\"white\"><B>Inodo " << inodeNum
          << " (" << typeStr << ")</B></FONT></TD></TR>\n";
    nodes << "      <TR><TD BGCOLOR=\"" << bgLight << "\"><B>Perm</B></TD><TD>"
          << inode.i_perm << "</TD></TR>\n";
    nodes << "      <TR><TD BGCOLOR=\"" << bgLight << "\"><B>UID</B></TD><TD>"
          << inode.i_uid << "</TD></TR>\n";
    nodes << "      <TR><TD BGCOLOR=\"" << bgLight << "\"><B>Size</B></TD><TD>"
          << inode.i_size << "</TD></TR>\n";
    for (int b = 0; b < 12; b++) {
        if (inode.i_block[b] == -1) continue;
        nodes << "      <TR><TD BGCOLOR=\"" << bgLight << "\"><B>B[" << b << "]</B></TD>"
              << "<TD>" << inode.i_block[b] << "</TD></TR>\n";
    }
    nodes << "    </TABLE>>];\n\n";

    // Procesar bloques
    for (int b = 0; b < 12; b++) {
        if (inode.i_block[b] == -1) continue;
        int blockNum = inode.i_block[b];
        string blockId = "block" + to_string(blockNum);

        edges << "  " << nodeId << " -> " << blockId << ";\n";

        if (inode.i_type == '0') {
            renderDirBlock(disk, sb, blockNum, inodeNum, nodes, edges);
        } else {
            renderFileBlock(disk, sb, blockNum, nodes);
        }
    }
}

void ReportTree::generate(FILE* disk, MountedPartition* part,
                           const string& path) {
    SuperBlock sb;
    readSuperBlock(disk, part->start, sb);

    visitedInodes.clear();
    visitedBlocks.clear();

    ostringstream nodes;
    ostringstream edges;

    renderInode(disk, sb, 0, nodes, edges);

    ostringstream dot;
    dot << "digraph TREE {\n";
    dot << "  node [shape=plaintext]\n";
    dot << "  rankdir=LR\n\n";
    dot << nodes.str();
    dot << edges.str();
    dot << "}\n";

    renderDot(dot.str(), path);
}
