#include "report_mbr.h"
#include "report_utils.h"
#include "../core/disk/mbr.h"
#include "../core/disk/ebr.h"
#include <sstream>
#include <cstring>
#include <ctime>
using namespace std;

void ReportMbr::generate(FILE* disk, MountedPartition* part,
                          const string& path) {
    MBR mbr;
    fseek(disk, 0, SEEK_SET);
    fread(&mbr, sizeof(MBR), 1, disk);

    ostringstream dot;
    dot << "digraph MBR {\n";
    dot << "  node [shape=plaintext]\n";
    dot << "  rankdir=TB\n\n";

    // Tabla MBR
    dot << "  mbr [label=<\n";
    dot << "    <TABLE BORDER=\"1\" CELLBORDER=\"1\" CELLSPACING=\"0\" BGCOLOR=\"#2196F3\">\n";
    dot << "      <TR><TD COLSPAN='2' BGCOLOR=\"#1565C0\"><FONT COLOR='white'><B>MBR</B></FONT></TD></TR>\n";
    dot << "      <TR><TD BGCOLOR=\"#E3F2FD\"><B>Tamaño</B></TD><TD>" << mbr.mbr_tamano << " bytes</TD></TR>\n";

    char timeBuf[64];
    struct tm* tm_info = localtime(&mbr.mbr_fecha_creacion);
    strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", tm_info);
    dot << "      <TR><TD BGCOLOR=\"#E3F2FD\"><B>Fecha Creación</B></TD><TD>" << timeBuf << "</TD></TR>\n";
    dot << "      <TR><TD BGCOLOR=\"#E3F2FD\"><B>Fit</B></TD><TD>" << mbr.dsk_fit << "</TD></TR>\n";

    // Particiones
    string colors[] = {"#FF9800","#4CAF50","#9C27B0","#F44336"};
    for (int i = 0; i < 4; i++) {
        if (mbr.mbr_partitions[i].part_size <= 0) continue;
        string type(1, mbr.mbr_partitions[i].part_type);
        string name(mbr.mbr_partitions[i].part_name);
        string status(1, mbr.mbr_partitions[i].part_status);
        string fit(mbr.mbr_partitions[i].part_fit, 3);

        dot << "      <TR><TD COLSPAN='2' BGCOLOR=\"" << colors[i] << "\">"
            << "<FONT COLOR='white'><B>Partición " << (i + 1) << ": " << name
            << "</B></FONT></TD></TR>\n";
        dot << "      <TR><TD BGCOLOR=\"#FFF8E1\"><B>Tipo</B></TD><TD>" << type << "</TD></TR>\n";
        dot << "      <TR><TD BGCOLOR=\"#FFF8E1\"><B>Fit</B></TD><TD>" << fit << "</TD></TR>\n";
        dot << "      <TR><TD BGCOLOR=\"#FFF8E1\"><B>Start</B></TD><TD>" << mbr.mbr_partitions[i].part_start << "</TD></TR>\n";
        dot << "      <TR><TD BGCOLOR=\"#FFF8E1\"><B>Size</B></TD><TD>" << mbr.mbr_partitions[i].part_size << " bytes</TD></TR>\n";
        dot << "      <TR><TD BGCOLOR=\"#FFF8E1\"><B>Status</B></TD><TD>" << status << "</TD></TR>\n";
    }

    dot << "    </TABLE>>];\n\n";

    // EBRs si hay partición extendida
    for (int i = 0; i < 4; i++) {
        if (mbr.mbr_partitions[i].part_type != 'E') continue;

        long long ebrPos = mbr.mbr_partitions[i].part_start;
        int ebrCount = 0;
        string prevNode = "mbr";

        while (ebrPos != -1 && ebrPos != 0) {
            EBR ebr;
            fseek(disk, ebrPos, SEEK_SET);
            fread(&ebr, sizeof(EBR), 1, disk);

            if (ebr.part_size <= 0) break;

            string ebrName = "ebr" + to_string(ebrCount++);
            string partName(ebr.part_name);
            string partFit(ebr.part_fit, 3);

            dot << "  " << ebrName << " [label=<\n";
            dot << "    <TABLE BORDER=\"1\" CELLBORDER=\"1\" CELLSPACING=\"0\" BGCOLOR=\"#009688\">\n";
            dot << "      <TR><TD COLSPAN='2' BGCOLOR=\"#00695C\"><FONT COLOR='white'><B>EBR: " << partName << "</B></FONT></TD></TR>\n";
            dot << "      <TR><TD BGCOLOR=\"#E0F2F1\"><B>Fit</B></TD><TD>" << partFit << "</TD></TR>\n";
            dot << "      <TR><TD BGCOLOR=\"#E0F2F1\"><B>Start</B></TD><TD>" << ebr.part_start << "</TD></TR>\n";
            dot << "      <TR><TD BGCOLOR=\"#b4cecc\"><B>Size</B></TD><TD>" << ebr.part_size << "</TD></TR>\n";
            dot << "      <TR><TD BGCOLOR=\"#E0F2F1\"><B>Next</B></TD><TD>" << ebr.part_next << "</TD></TR>\n";
            dot << "    </TABLE>>];\n\n";

            dot << "  " << prevNode << " -> " << ebrName << ";\n";
            prevNode = ebrName;

            ebrPos = ebr.part_next;
        }
    }

    dot << "}\n";
    renderDot(dot.str(), path);
}
