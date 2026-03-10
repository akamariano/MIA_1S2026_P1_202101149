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

    char timeBuf[64];
    struct tm* tm_info = localtime(&mbr.mbr_fecha_creacion);
    strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", tm_info);

    char fitBuf[4] = {0};
    strncpy(fitBuf, mbr.dsk_fit, 3);

    ostringstream dot;
    dot << "digraph MBR {\n";
    dot << "  node [shape=plaintext fontname=\"Helvetica\"]\n";
    dot << "  rankdir=TB\n\n";

    dot << "  mbr [label=<\n";
    dot << "    <TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"6\" WIDTH=\"350\">\n";

    // Encabezado del reporte
    dot << "      <TR><TD COLSPAN=\"2\" BGCOLOR=\"#4A235A\" ALIGN=\"CENTER\">"
        << "<FONT COLOR=\"white\"><B>REPORTE DE MBR</B></FONT></TD></TR>\n";

    // Información principal del MBR
    dot << "      <TR><TD BGCOLOR=\"#D7BDE2\" ALIGN=\"LEFT\"><B>mbr_tamano</B></TD>"
        << "<TD BGCOLOR=\"#F5EEF8\" ALIGN=\"LEFT\">" << mbr.mbr_tamano << "</TD></TR>\n";
    dot << "      <TR><TD BGCOLOR=\"#D7BDE2\" ALIGN=\"LEFT\"><B>mbr_fecha_creacion</B></TD>"
        << "<TD BGCOLOR=\"#F5EEF8\" ALIGN=\"LEFT\">" << timeBuf << "</TD></TR>\n";
    dot << "      <TR><TD BGCOLOR=\"#D7BDE2\" ALIGN=\"LEFT\"><B>mbr_dsk_signature</B></TD>"
        << "<TD BGCOLOR=\"#F5EEF8\" ALIGN=\"LEFT\">" << mbr.mbr_dsk_signature << "</TD></TR>\n";

    // Mostrar todas las particiones primarias
    for (int i = 0; i < 4; i++) {
        auto& p = mbr.mbr_partitions[i];
        if (p.part_size <= 0) continue;

        char pFit[4] = {0};
        strncpy(pFit, p.part_fit, 3);
        string pName(p.part_name);
        string pType(1, p.part_type);
        string pStatus(1, p.part_status);

        // Encabezado para esta partición
        dot << "      <TR><TD COLSPAN=\"2\" BGCOLOR=\"#6C3483\" ALIGN=\"CENTER\">"
            << "<FONT COLOR=\"white\"><B>Particion " << (i+1) << "</B></FONT></TD></TR>\n";

        // Detalles de la partición primaria
        dot << "      <TR><TD BGCOLOR=\"#D2B4DE\" ALIGN=\"LEFT\"><B>part_status</B></TD>"
            << "<TD BGCOLOR=\"#F5EEF8\" ALIGN=\"LEFT\">" << pStatus << "</TD></TR>\n";
        dot << "      <TR><TD BGCOLOR=\"#D2B4DE\" ALIGN=\"LEFT\"><B>part_type</B></TD>"
            << "<TD BGCOLOR=\"#F5EEF8\" ALIGN=\"LEFT\">" << pType << "</TD></TR>\n";
        dot << "      <TR><TD BGCOLOR=\"#D2B4DE\" ALIGN=\"LEFT\"><B>part_fit</B></TD>"
            << "<TD BGCOLOR=\"#F5EEF8\" ALIGN=\"LEFT\">" << pFit << "</TD></TR>\n";
        dot << "      <TR><TD BGCOLOR=\"#D2B4DE\" ALIGN=\"LEFT\"><B>part_start</B></TD>"
            << "<TD BGCOLOR=\"#F5EEF8\" ALIGN=\"LEFT\">" << p.part_start << "</TD></TR>\n";
        dot << "      <TR><TD BGCOLOR=\"#D2B4DE\" ALIGN=\"LEFT\"><B>part_size</B></TD>"
            << "<TD BGCOLOR=\"#F5EEF8\" ALIGN=\"LEFT\">" << p.part_size << "</TD></TR>\n";
        dot << "      <TR><TD BGCOLOR=\"#D2B4DE\" ALIGN=\"LEFT\"><B>part_name</B></TD>"
            << "<TD BGCOLOR=\"#F5EEF8\" ALIGN=\"LEFT\">" << pName << "</TD></TR>\n";

        // Si la partición es extendida, mostrar sus particiones lógicas usando EBR
        if (p.part_type == 'E') {
            long long ebrPos = p.part_start;
while (ebrPos > 0 && ebrPos < (long long)mbr.mbr_tamano) {
    EBR ebr;
    memset(&ebr, 0, sizeof(EBR));
    fseek(disk, (long)ebrPos, SEEK_SET);
    fread(&ebr, sizeof(EBR), 1, disk);
    if (ebr.part_size <= 0) break;

    char eFit[4] = {0};
    strncpy(eFit, ebr.part_fit, 3);
    string eName(ebr.part_name, strnlen(ebr.part_name, 16));

    // Mostrar partición lógica
    dot << "      <TR><TD COLSPAN=\"2\" BGCOLOR=\"#A569BD\" ALIGN=\"CENTER\">"
        << "<FONT COLOR=\"white\"><B>Particion Logica</B></FONT></TD></TR>\n";
    dot << "      <TR><TD BGCOLOR=\"#E8DAEF\" ALIGN=\"LEFT\"><B>part_mount</B></TD>"
        << "<TD BGCOLOR=\"#FAF5FF\" ALIGN=\"LEFT\">" << ebr.part_mount << "</TD></TR>\n";
    dot << "      <TR><TD BGCOLOR=\"#E8DAEF\" ALIGN=\"LEFT\"><B>part_next</B></TD>"
        << "<TD BGCOLOR=\"#FAF5FF\" ALIGN=\"LEFT\">" << ebr.part_next << "</TD></TR>\n";
    dot << "      <TR><TD BGCOLOR=\"#E8DAEF\" ALIGN=\"LEFT\"><B>part_fit</B></TD>"
        << "<TD BGCOLOR=\"#FAF5FF\" ALIGN=\"LEFT\">" << eFit << "</TD></TR>\n";
    dot << "      <TR><TD BGCOLOR=\"#E8DAEF\" ALIGN=\"LEFT\"><B>part_start</B></TD>"
        << "<TD BGCOLOR=\"#FAF5FF\" ALIGN=\"LEFT\">" << ebr.part_start << "</TD></TR>\n";
    dot << "      <TR><TD BGCOLOR=\"#E8DAEF\" ALIGN=\"LEFT\"><B>part_size</B></TD>"
        << "<TD BGCOLOR=\"#FAF5FF\" ALIGN=\"LEFT\">" << ebr.part_size << "</TD></TR>\n";
    dot << "      <TR><TD BGCOLOR=\"#E8DAEF\" ALIGN=\"LEFT\"><B>part_name</B></TD>"
        << "<TD BGCOLOR=\"#FAF5FF\" ALIGN=\"LEFT\">" << eName << "</TD></TR>\n";
    dot << "      <TR><TD BGCOLOR=\"#E8DAEF\" ALIGN=\"LEFT\"><B>part_correlative</B></TD>"
    << "<TD BGCOLOR=\"#FAF5FF\" ALIGN=\"LEFT\">" << (int)ebr.part_correlative << "</TD></TR>\n";

    // Iterar al siguiente EBR en la cadena
    long long nextPos = ebr.part_next;
    if (nextPos == ebrPos) break; // seguridad: evitar loop infinito
    ebrPos = nextPos;
}
        }
    }

    // Pie de página del reporte
    dot << "      <TR><TD COLSPAN=\"2\" BGCOLOR=\"#4A235A\" ALIGN=\"CENTER\">"
        << "<FONT COLOR=\"white\"><FONT POINT-SIZE=\"9\">Reporte de MBR</FONT></FONT></TD></TR>\n";

    dot << "    </TABLE>>];\n";
    dot << "}\n";

    renderDot(dot.str(), path);
}