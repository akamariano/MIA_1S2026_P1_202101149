#include "report_sb.h"
#include "report_utils.h"
#include "../core/filesystem/EXT2Utils.h"
#include "../core/filesystem/SuperBlock.h"
#include <sstream>
#include <iomanip>
#include <ctime>
using namespace std;

void ReportSb::generate(FILE* disk, MountedPartition* part, const string& path) {
    SuperBlock sb;
    readSuperBlock(disk, part->start, sb);

    auto fmtTime = [](time_t t) -> string {
        if (t == 0) return "N/A";
        char buf[32];
        struct tm* ti = localtime(&t);
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ti);
        return string(buf);
    };

    auto fmtHex = [](int v) -> string {
        ostringstream s;
        s << "0x" << hex << uppercase << v;
        return s.str();
    };

    string diskName = part->path.substr(part->path.find_last_of("/\\") + 1);

    ostringstream dot;
    dot << "digraph SB {\n";
    dot << "  node [shape=plaintext fontname=\"Helvetica\"]\n\n";
    dot << "  sb [label=<\n";
    dot << "  <TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"6\" WIDTH=\"500\">\n";

    dot << "    <TR><TD COLSPAN=\"2\" BGCOLOR=\"#1E8449\" ALIGN=\"CENTER\">"
        << "<FONT COLOR=\"white\"><B>Reporte de SUPERBLOQUE</B></FONT></TD></TR>\n";

    auto row = [&](const string& name, const string& value, bool dark) {
        string bg = dark ? "#A9DFBF" : "#D5F5E3";
        dot << "    <TR>"
            << "<TD BGCOLOR=\"" << bg << "\" ALIGN=\"LEFT\"><B>" << name << "</B></TD>"
            << "<TD BGCOLOR=\"" << bg << "\" ALIGN=\"LEFT\">" << value << "</TD>"
            << "</TR>\n";
    };

    bool d = false;
    row("Disco",              diskName,                              d=!d);
    row("s_filesystem_type",  to_string(sb.s_filesystem_type),      d=!d);
    row("s_inodes_count",     to_string(sb.s_inodes_count),         d=!d);
    row("s_blocks_count",     to_string(sb.s_blocks_count),         d=!d);
    row("s_free_blocks_count",to_string(sb.s_free_blocks_count),    d=!d);
    row("s_free_inodes_count",to_string(sb.s_free_inodes_count),    d=!d);
    row("s_mtime",            fmtTime(sb.s_mtime),                   d=!d);
    row("s_umtime",           fmtTime(sb.s_umtime),                  d=!d);
    row("s_mnt_count",        to_string(sb.s_mnt_count),            d=!d);
    row("s_magic",            fmtHex(sb.s_magic),                    d=!d);
    row("s_inode_s",          to_string(sb.s_inode_size),            d=!d);
    row("s_block_s",          to_string(sb.s_block_size),            d=!d);
    row("s_firts_ino",        to_string(sb.s_first_ino),             d=!d);
    row("s_first_blo",        to_string(sb.s_first_blo),             d=!d);
    row("s_bm_inode_start",   to_string(sb.s_bm_inode_start),       d=!d);
    row("s_bm_block_start",   to_string(sb.s_bm_block_start),       d=!d);
    row("s_inode_start",      to_string(sb.s_inode_start),          d=!d);
    row("s_block_start",      to_string(sb.s_block_start),          d=!d);

    dot << "    <TR><TD COLSPAN=\"2\" BGCOLOR=\"#1E8449\" ALIGN=\"CENTER\">"
        << "<FONT COLOR=\"white\">Reporte de SuperBloque</FONT></TD></TR>\n";

    dot << "  </TABLE>>];\n";
    dot << "}\n";

    renderDot(dot.str(), path);
}
