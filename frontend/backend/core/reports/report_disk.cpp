#include "report_disk.h"
#include "report_utils.h"
#include "../core/disk/mbr.h"
#include "../core/disk/ebr.h"
#include <sstream>
#include <cstring>
#include <iomanip>
#include <vector>
#include <algorithm>
using namespace std;

void ReportDisk::generate(FILE* disk, MountedPartition* part, const string& path) {
    MBR mbr;
    fseek(disk, 0, SEEK_SET);
    fread(&mbr, sizeof(MBR), 1, disk);

    long long totalSize = mbr.mbr_tamano;
    string diskName = part->path.substr(part->path.find_last_of("/\\") + 1);

    struct Segment {
        string type;   // "MBR","FREE","PRIMARY","EXTENDED","EBR","LOGICAL"
        string label;
        string sublabel;
        long long size;
    };

    // Recolectar particiones ordenadas
    struct PartInfo {
        long long start, size;
        char type;
        string name;
    };
    vector<PartInfo> parts;
    for (int i = 0; i < 4; i++) {
        auto& p = mbr.mbr_partitions[i];
        if (p.part_size <= 0) continue;
        parts.push_back({p.part_start, p.part_size, p.part_type, string(p.part_name)});
    }
    sort(parts.begin(), parts.end(), [](const PartInfo& a, const PartInfo& b){
        return a.start < b.start;
    });

    // Calcular ancho proporcional (mínimo 8%)
    auto pctStr = [&](long long sz) -> string {
        double p = (double)sz / totalSize * 100.0;
        ostringstream s;
        s << fixed << setprecision(0) << p << "% del disco";
        return s.str();
    };

    // Generar DOT con HTML table anidada para extendida
    ostringstream dot;
    dot << "digraph DISK {\n";
    dot << "  node [shape=plaintext fontname=\"Helvetica\" margin=0]\n\n";

    dot << "  disk [label=<\n";
    dot << "  <TABLE BORDER=\"1\" CELLBORDER=\"0\" CELLSPACING=\"0\" CELLPADDING=\"0\">\n";
    // Título
    dot << "    <TR><TD COLSPAN=\"100\" ALIGN=\"CENTER\" CELLPADDING=\"4\">"
        << "<B>" << diskName << "</B></TD></TR>\n";
    dot << "    <TR>\n";

    long long cursor = 0;

    // MBR fijo
    dot << "      <TD BORDER=\"1\" CELLPADDING=\"8\" ALIGN=\"CENTER\" WIDTH=\"50\">"
        << "<B>MBR</B></TD>\n";
    cursor = sizeof(MBR);

    for (auto& p : parts) {
        // Espacio libre antes
        if (p.start > cursor) {
            long long freeSize = p.start - cursor;
            dot << "      <TD BORDER=\"1\" CELLPADDING=\"8\" ALIGN=\"CENTER\" WIDTH=\"80\">"
                << "Libre<BR/><FONT POINT-SIZE=\"9\">" << pctStr(freeSize) << "</FONT></TD>\n";
        }

        if (p.type == 'E') {
            // Partición extendida — tabla anidada
            dot << "      <TD BORDER=\"1\" CELLPADDING=\"0\" ALIGN=\"CENTER\">\n";
            dot << "        <TABLE BORDER=\"0\" CELLBORDER=\"0\" CELLSPACING=\"0\" CELLPADDING=\"0\">\n";
            dot << "          <TR><TD COLSPAN=\"100\" ALIGN=\"CENTER\" CELLPADDING=\"2\">"
                << "<B>Extendida</B></TD></TR>\n";
            dot << "          <TR>\n";

            long long ebrPos = p.start;
            long long extCursor = p.start;

            while (ebrPos > 0 && ebrPos < (long long)mbr.mbr_tamano) {
                EBR ebr;
                memset(&ebr, 0, sizeof(EBR));
                fseek(disk, (long)ebrPos, SEEK_SET);
                fread(&ebr, sizeof(EBR), 1, disk);
                if (ebr.part_size <= 0) break;

                // Espacio libre antes del EBR
                if (ebrPos > extCursor) {
                    long long fs = ebrPos - extCursor;
                    dot << "            <TD BORDER=\"1\" CELLPADDING=\"6\" ALIGN=\"CENTER\" WIDTH=\"60\">"
                        << "Libre<BR/><FONT POINT-SIZE=\"8\">" << pctStr(fs) << "</FONT></TD>\n";
                }

                // EBR
                dot << "            <TD BORDER=\"1\" CELLPADDING=\"6\" ALIGN=\"CENTER\" WIDTH=\"40\">"
                    << "<B>EBR</B></TD>\n";

                // Lógica
                string ename(ebr.part_name, strnlen(ebr.part_name, 16));
                dot << "            <TD BORDER=\"1\" CELLPADDING=\"6\" ALIGN=\"CENTER\" WIDTH=\"80\">"
                    << "Lógica<BR/><B>" << ename << "</B><BR/>"
                    << "<FONT POINT-SIZE=\"8\">" << pctStr(ebr.part_size) << "</FONT></TD>\n";

                extCursor = ebr.part_start + ebr.part_size;

                long long nextPos = ebr.part_next;
                if (nextPos <= 0 || nextPos == ebrPos) break;
                ebrPos = nextPos;
            }

            // Libre al final de extendida
            long long extEnd = p.start + p.size;
            if (extCursor < extEnd) {
                long long fs = extEnd - extCursor;
                dot << "            <TD BORDER=\"1\" CELLPADDING=\"6\" ALIGN=\"CENTER\" WIDTH=\"60\">"
                    << "Libre<BR/><FONT POINT-SIZE=\"8\">" << pctStr(fs) << "</FONT></TD>\n";
            }

            dot << "          </TR>\n";
            dot << "        </TABLE>\n";
            dot << "      </TD>\n";

        } else {
            // Primaria
            dot << "      <TD BORDER=\"1\" CELLPADDING=\"8\" ALIGN=\"CENTER\" WIDTH=\"100\">"
                << "Primaria<BR/><B>" << p.name << "</B><BR/>"
                << "<FONT POINT-SIZE=\"9\">" << pctStr(p.size) << "</FONT></TD>\n";
        }

        cursor = p.start + p.size;
    }

    // Libre al final
    if (cursor < totalSize) {
        long long freeSize = totalSize - cursor;
        dot << "      <TD BORDER=\"1\" CELLPADDING=\"8\" ALIGN=\"CENTER\" WIDTH=\"80\">"
            << "Libre<BR/><FONT POINT-SIZE=\"9\">" << pctStr(freeSize) << "</FONT></TD>\n";
    }

    dot << "    </TR>\n";
    dot << "  </TABLE>>];\n";
    dot << "}\n";

    renderDot(dot.str(), path);
}
