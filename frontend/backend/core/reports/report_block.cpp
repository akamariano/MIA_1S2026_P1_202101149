#include "report_block.h"
#include "report_utils.h"
#include "../core/filesystem/EXT2Utils.h"
#include "../core/filesystem/SuperBlock.h"
#include "../core/filesystem/Inode.h"
#include "../core/filesystem/Blocks.h"
#include <sstream>
#include <cstring>
#include <set>
using namespace std;

void ReportBlock::generate(FILE* disk, MountedPartition* part,
                            const string& path) {
    SuperBlock sb;
    readSuperBlock(disk, part->start, sb);

    // Primero identificar qué bloques son apuntadores (i_block[12])
    set<int> pointerBlocks;
    for (int i = 0; i < sb.s_inodes_count; i++) {
        fseek(disk, sb.s_bm_inode_start + i, SEEK_SET);
        unsigned char used;
        fread(&used, 1, 1, disk);
        if (!used) continue;

        Inode inode;
        readInode(disk, sb, i, inode);
        if (inode.i_block[12] != -1)
            pointerBlocks.insert(inode.i_block[12]);
    }

    ostringstream nodes, edges;
    string prevNode = "";

    for (int i = 0; i < sb.s_blocks_count; i++) {
        fseek(disk, sb.s_bm_block_start + i, SEEK_SET);
        unsigned char used;
        fread(&used, 1, 1, disk);
        if (!used) continue;

        string nodeId = "block" + to_string(i);

        // ── Bloque Apuntador ──
        if (pointerBlocks.count(i)) {
            PointerBlock pb;
            memset(&pb, 0, sizeof(PointerBlock));
            readBlock(disk, sb, i, &pb);

            nodes << "  " << nodeId << " [label=<\n";
            nodes << "    <TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\""
                  << " CELLPADDING=\"4\" WIDTH=\"180\">\n";
            nodes << "      <TR><TD BGCOLOR=\"#FCF3CF\" ALIGN=\"CENTER\">"
                  << "<B>Bloque Apuntadores " << (i + 1) << "</B></TD></TR>\n";

            // Mostrar todos los punteros
            string ptrs;
            for (int p = 0; p < 16; p++) {
                if (p > 0) ptrs += ", ";
                ptrs += to_string(pb.b_pointers[p]);
            }
            nodes << "      <TR><TD BGCOLOR=\"#FDFEFE\">" << ptrs << "</TD></TR>\n";
            nodes << "    </TABLE>>];\n\n";

        } else {
            // Leer como DirectoryBlock para detectar tipo
            DirectoryBlock db;
            memset(&db, 0, sizeof(DirectoryBlock));
            readBlock(disk, sb, i, &db);

            bool isDir = false;
            int validEntries = 0;
            for (int e = 0; e < 4; e++) {
                int einode = db.b_content[e].b_inodo;
                if (einode == -1) { validEntries++; continue; }
                if (einode < 0 || einode >= sb.s_inodes_count) continue;

                bool validName = false;
                for (int c = 0; c < 12; c++) {
                    unsigned char ch = db.b_content[e].b_name[c];
                    if (ch == 0) break;
                    if (ch >= 32 && ch < 127) { validName = true; }
                    else { validName = false; break; }
                }
                if (validName) { validEntries++; isDir = true; }
            }
            if (validEntries < 2) isDir = false;

            if (isDir) {
                // ── Bloque Carpeta ──
                nodes << "  " << nodeId << " [label=<\n";
                nodes << "    <TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\""
                      << " CELLPADDING=\"5\" WIDTH=\"180\">\n";
                nodes << "      <TR><TD COLSPAN=\"2\" BGCOLOR=\"#D5D8DC\" ALIGN=\"CENTER\">"
                      << "<B>Bloque Carpeta " << (i + 1) << "</B></TD></TR>\n";
                nodes << "      <TR>"
                      << "<TD BGCOLOR=\"#EBF5FB\"><B>b_name</B></TD>"
                      << "<TD BGCOLOR=\"#EBF5FB\"><B>b_inodo</B></TD>"
                      << "</TR>\n";

                for (int e = 0; e < 4; e++) {
                    string ename(db.b_content[e].b_name,
                                 strnlen(db.b_content[e].b_name, 12));
                    string einode = (db.b_content[e].b_inodo == -1)
                                    ? "-1"
                                    : to_string(db.b_content[e].b_inodo);
                    nodes << "      <TR>"
                          << "<TD BGCOLOR=\"#FDFEFE\">" << (ename.empty() ? "-" : ename) << "</TD>"
                          << "<TD BGCOLOR=\"#FDFEFE\">" << einode << "</TD>"
                          << "</TR>\n";
                }
                nodes << "    </TABLE>>];\n\n";

            } else {
                // ── Bloque Archivo ──
                FileBlock fb;
                memset(&fb, 0, sizeof(FileBlock));
                readBlock(disk, sb, i, &fb);

                string content;
                for (int c = 0; c < 64; c++) {
                    unsigned char ch = fb.b_content[c];
                    if (ch == 0) break;
                    if      (ch == '<')  content += "&lt;";
                    else if (ch == '>')  content += "&gt;";
                    else if (ch == '&')  content += "&amp;";
                    else if (ch == '"')  content += "&quot;";
                    else if (ch == '\n') content += " ";
                    else if (ch >= 32 && ch < 127) content += (char)ch;
                    else content += '.';
                }
                if (content.size() > 48)
                    content = content.substr(0, 48) + "...";

                nodes << "  " << nodeId << " [label=<\n";
                nodes << "    <TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\""
                      << " CELLPADDING=\"5\" WIDTH=\"220\">\n";
                nodes << "      <TR><TD BGCOLOR=\"#D5D8DC\" ALIGN=\"CENTER\">"
                      << "<B>Bloque Archivo " << (i + 1) << "</B></TD></TR>\n";
                nodes << "      <TR><TD BGCOLOR=\"#FDFEFE\">"
                      << (content.empty() ? "-" : content)
                      << "</TD></TR>\n";
                nodes << "    </TABLE>>];\n\n";
            }
        }

        if (!prevNode.empty())
            edges << "  " << prevNode << " -> " << nodeId << ";\n";
        prevNode = nodeId;
    }

    ostringstream dot;
    dot << "digraph BLOCKS {\n";
    dot << "  node [shape=plaintext fontname=\"Helvetica\"]\n";
    dot << "  rankdir=LR\n\n";
    dot << nodes.str();
    dot << edges.str();
    dot << "}\n";

    renderDot(dot.str(), path);
}