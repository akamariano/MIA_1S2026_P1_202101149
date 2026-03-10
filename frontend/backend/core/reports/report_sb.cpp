#include "report_sb.h"
#include "report_utils.h"
#include "../core/filesystem/EXT2Utils.h"
#include "../core/filesystem/SuperBlock.h"
#include <sstream>
#include <ctime>
#include <iomanip>
using namespace std;

void ReportSb::generate(FILE* disk, MountedPartition* part,
                         const string& path) {
    SuperBlock sb;
    readSuperBlock(disk, part->start, sb);

    auto fmtTime = [](time_t t) -> string {
        if (t == 0) return "N/A";
        char buf[32];
        struct tm* ti = localtime(&t);
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ti);
        return string(buf);
    };

    ostringstream dot;
    dot << "digraph SB {\n";
    dot << "  node [shape=plaintext fontname=\"Helvetica\"]\n\n";

    dot << "  sb [label=<\n";
    dot << "    <TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"6\" WIDTH=\"400\">\n";

    // Header
    dot << "      <TR><TD COLSPAN=\"2\" BGCOLOR=\"#1E8449\" ALIGN=\"CENTER\">"
        << "<FONT COLOR=\"white\"><B>Reporte de SUPERBLOQUE</B></FONT></TD></TR>\n";

    auto row = [&](const string& key, const string& val) {
        dot << "      <TR>"
            << "<TD BGCOLOR=\"#A9DFBF\" ALIGN=\"LEFT\"><B>" << key << "</B></TD>"
            << "<TD BGCOLOR=\"#D5F5E3\" ALIGN=\"LEFT\">" << val << "</TD>"
            << "</TR>\n";
    };

    row("sb_nombre_hd",               part->path.substr(part->path.find_last_of("/\\") + 1));
    row("sb_ap_inodo_s",              to_string(sb.s_inodes_count));
    row("sb_ap_bloque_s",             to_string(sb.s_blocks_count));
    row("sb_inodos_libres",           to_string(sb.s_free_inodes_count));
    row("sb_bloques_libres",          to_string(sb.s_free_blocks_count));
    row("sb_detalle_directorio_free", to_string(sb.s_free_inodes_count));
    row("sb_bloques_free",            to_string(sb.s_free_blocks_count));
    row("sb_fecha_creacion",          fmtTime(sb.s_mtime));
    row("sb_fecha_ultimo_montaje",    fmtTime(sb.s_mtime));
    row("sb_magic",                   "0xef53");
    row("sb_ap_bitmap_arbol_directorio", to_string(sb.s_bm_inode_start));
    row("sb_ap_arbol_directorio",     to_string(sb.s_inode_start));
    row("sb_ap_bitmap_directorio",    to_string(sb.s_bm_inode_start));
    row("sb_ap_detalle_directorio",   to_string(sb.s_inode_start));
    row("sb_ap_bitmap_bloques",       to_string(sb.s_bm_block_start));
    row("sb_ap_bloques",              to_string(sb.s_block_start));
    row("sb_tamano_inodo",            to_string(sb.s_inode_size));
    row("sb_size_struct_arbol_directorio", to_string(sb.s_inode_size));
    row("sb_size_directorio_detalle_director", to_string(sizeof(DirectoryBlock)));
    row("sb_size_free_bit_bloque",    "1");
    row("sb_first_free_bit_inode_directorio", to_string(sb.s_first_ino));
    row("sb_first_free_bit_bloques", to_string(sb.s_first_blo));
    row("sb_magic_2",                 "0xef53");

    // Footer
    dot << "      <TR><TD COLSPAN=\"2\" BGCOLOR=\"#1E8449\" ALIGN=\"CENTER\">"
        << "<FONT COLOR=\"white\"><FONT POINT-SIZE=\"9\">Reporte de SuperBloque</FONT></FONT></TD></TR>\n";

    dot << "    </TABLE>>];\n";
    dot << "}\n";

    renderDot(dot.str(), path);
}
