#include "report_sb.h"
#include "report_utils.h"
#include "../core/filesystem/EXT2Utils.h"
#include "../core/filesystem/SuperBlock.h"
#include <sstream>
#include <ctime>
using namespace std;

void ReportSb::generate(FILE* disk, MountedPartition* part,
                         const string& path) {
    SuperBlock sb;
    readSuperBlock(disk, part->start, sb);

    auto fmtTime = [](time_t t) -> string {
        if (t == 0) return "N/A";
        char buf[64];
        struct tm* tm_info = localtime(&t);
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);
        return string(buf);
    };

    ostringstream dot;
    dot << "digraph SB {\n";
    dot << "  node [shape=plaintext]\n\n";
    dot << "  sb [label=<\n";
    dot << "    <TABLE BORDER='1' CELLBORDER='1' CELLSPACING='0'>\n";
    dot << "      <TR><TD COLSPAN='2' BGCOLOR='#1A237E'><FONT COLOR='white'><B>SUPERBLOQUE</B></FONT></TD></TR>\n";

    auto row = [&](const string& field, const string& val) {
        dot << "      <TR><TD BGCOLOR='#E8EAF6'><B>" << field
            << "</B></TD><TD>" << val << "</TD></TR>\n";
    };

    row("Filesystem Type",    to_string(sb.s_filesystem_type));
    row("Inodos Totales",     to_string(sb.s_inodes_count));
    row("Bloques Totales",    to_string(sb.s_blocks_count));
    row("Inodos Libres",      to_string(sb.s_free_inodes_count));
    row("Bloques Libres",     to_string(sb.s_free_blocks_count));
    row("Tamaño Inodo",       to_string(sb.s_inode_size) + " bytes");
    row("Tamaño Bloque",      to_string(sb.s_block_size) + " bytes");
    row("Magic",              "0x" + [&](){
                                  ostringstream ss;
                                  ss << hex << sb.s_magic;
                                  return ss.str(); }());
    row("Primer Inodo",       to_string(sb.s_first_ino));
    row("Primer Bloque",      to_string(sb.s_first_blo));
    row("Último Montaje",     fmtTime(sb.s_mtime));
    row("Último Desmontaje",  fmtTime(sb.s_umtime));
    row("Veces Montado",      to_string(sb.s_mnt_count));
    row("BM Inodos Start",    to_string(sb.s_bm_inode_start));
    row("BM Bloques Start",   to_string(sb.s_bm_block_start));
    row("Inodos Start",       to_string(sb.s_inode_start));
    row("Bloques Start",      to_string(sb.s_block_start));

    dot << "    </TABLE>>];\n";
    dot << "}\n";

    renderDot(dot.str(), path);
}