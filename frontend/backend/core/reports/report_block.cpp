#include "report_block.h"
#include "report_utils.h"
#include "../core/filesystem/EXT2Utils.h"
#include "../core/filesystem/SuperBlock.h"
#include "../core/filesystem/Inode.h"
#include "../core/filesystem/Blocks.h"
#include <sstream>
#include <cstring>
using namespace std;

void ReportBlock::generate(FILE* disk, MountedPartition* part,
                            const string& path) {
    SuperBlock sb;
    readSuperBlock(disk, part->start, sb);

    ostringstream nodes, edges;
    string prevNode = "";

    // Iterar sobre todos los bloques y analizar aquellos que están en uso
    for (int i = 0; i < sb.s_blocks_count; i++) {
        // Leer del mapa de bits para saber si el bloque está en uso
        fseek(disk, sb.s_bm_block_start + i, SEEK_SET);
        unsigned char used;
        fread(&used, 1, 1, disk);
        if (!used) continue;

        string nodeId = "block" + to_string(i);

        // Intentar interpretar como DirectoryBlock
        DirectoryBlock db;
        memset(&db, 0, sizeof(DirectoryBlock));
        readBlock(disk, sb, i, &db);

        // Detectar tipo de bloque: si contiene entradas válidas = es un directorio
        bool isDir = false;
        for (int e = 0; e < 4; e++) {
            if (db.b_content[e].b_inodo >= 0 &&
                db.b_content[e].b_inodo < sb.s_inodes_count) {
                for (int c = 0; c < 12 && db.b_content[e].b_name[c]; c++) {
                    if (db.b_content[e].b_name[c] >= 32 &&
                        db.b_content[e].b_name[c] < 127) {
                        isDir = true; break;
                    }
                }
                if (isDir) break;
            }
        }

        if (isDir) {
            // Bloque de directorio con entradas de archivos/carpetas
            nodes << "  " << nodeId << " [label=<\n";
            nodes << "    <TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"5\" WIDTH=\"200\">\n";
            nodes << "      <TR><TD COLSPAN=\"2\" BGCOLOR=\"#D5D8DC\" ALIGN=\"CENTER\">"
                  << "<B>Bloque Carpeta " << (i+1) << "</B></TD></TR>\n";
            nodes << "      <TR><TD BGCOLOR=\"#EBF5FB\"><B>b_name</B></TD>"
                  << "<TD BGCOLOR=\"#EBF5FB\"><B>b_inodo</B></TD></TR>\n";

            for (int e = 0; e < 4; e++) {
                string ename(db.b_content[e].b_name,
                             strnlen(db.b_content[e].b_name, 12));
                string einode = (db.b_content[e].b_inodo == -1) ? "-1" :
                                 to_string(db.b_content[e].b_inodo);
                nodes << "      <TR><TD BGCOLOR=\"#FDFEFE\">"
                      << (ename.empty() ? "-" : ename)
                      << "</TD><TD BGCOLOR=\"#FDFEFE\">" << einode
                      << "</TD></TR>\n";
            }
            nodes << "    </TABLE>>];\n\n";

        } else {
            // Intentar como PointerBlock para apuntadores indirectos
            PointerBlock pb;
            memset(&pb, 0, sizeof(PointerBlock));
            readBlock(disk, sb, i, &pb);

            // Detectar si es un bloque de apuntadores contando referencias válidas
            bool isPointer = false;
            int validPtrs = 0;
            for (int p = 0; p < 16; p++) {
                if (pb.b_pointers[p] >= 0 && pb.b_pointers[p] < sb.s_blocks_count)
                    validPtrs++;
            }
            if (validPtrs > 2) isPointer = true;

            if (isPointer) {
                // Bloque de apuntadores (apunta a otros bloques)
                nodes << "  " << nodeId << " [label=<\n";
                nodes << "    <TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"5\" WIDTH=\"200\">\n";
                nodes << "      <TR><TD BGCOLOR=\"#D5D8DC\" ALIGN=\"CENTER\">"
                      << "<B>Bloque Apuntadores " << (i+1) << "</B></TD></TR>\n";

                for (int p = 0; p < 16; p++) {
                    string val = (pb.b_pointers[p] == -1) ? "-1" :
                                  to_string(pb.b_pointers[p]);
                    nodes << "      <TR><TD BGCOLOR=\"#FEF9E7\">" << val << "</TD></TR>\n";
                }
                nodes << "    </TABLE>>];\n\n";

            } else {
                // Bloque de archivo con contenido de datos
                FileBlock fb;
                memset(&fb, 0, sizeof(FileBlock));
                readBlock(disk, sb, i, &fb);

                // Extraer contenido legible (escapar caracteres especiales HTML)
                string content;
                for (int c = 0; c < 64; c++) {
                    unsigned char ch = fb.b_content[c];
                    if (ch == 0) break;
                    if      (ch == '<')  content += "&lt;";
                    else if (ch == '>')  content += "&gt;";
                    else if (ch == '&')  content += "&amp;";
                    else if (ch == '\n') content += " ";
                    else if (ch >= 32 && ch < 127) content += (char)ch;
                }
                if (content.size() > 50) content = content.substr(0, 50) + "...";

                nodes << "  " << nodeId << " [label=<\n";
                nodes << "    <TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"5\" WIDTH=\"200\">\n";
                nodes << "      <TR><TD BGCOLOR=\"#D5D8DC\" ALIGN=\"CENTER\">"
                      << "<B>Bloque Archivo " << (i+1) << "</B></TD></TR>\n";
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
