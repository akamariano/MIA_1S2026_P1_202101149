#include "report_inode.h"
#include "report_utils.h"
#include "../core/filesystem/EXT2Utils.h"
#include "../core/filesystem/SuperBlock.h"
#include "../core/filesystem/Inode.h"
#include <sstream>
#include <cstring>
#include <ctime>
using namespace std;

void ReportInode::generate(FILE* disk, MountedPartition* part,
                            const string& path) {
    SuperBlock sb;
    readSuperBlock(disk, part->start, sb);

    auto fmtTime = [](time_t t) -> string {
        if (t == 0) return "N/A";
        char buf[32];
        struct tm* ti = localtime(&t);
        strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M", ti);
        return string(buf);
    };

    ostringstream nodes, edges;
    string prevNode = "";

    for (int i = 0; i < sb.s_inodes_count; i++) {
        fseek(disk, sb.s_bm_inode_start + i, SEEK_SET);
        unsigned char used;
        fread(&used, 1, 1, disk);
        if (!used) continue;

        Inode inode;
        readInode(disk, sb, i, inode);

        string nodeId = "inode" + to_string(i);

        nodes << "  " << nodeId << " [label=<\n";
        nodes << "    <TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"5\" WIDTH=\"180\">\n";
        nodes << "      <TR><TD COLSPAN=\"2\" BGCOLOR=\"#D5D8DC\" ALIGN=\"CENTER\">"
              << "<B>Inodo " << (i+1) << "</B></TD></TR>\n";

        auto row = [&](const string& k, const string& v) {
            nodes << "      <TR>"
                  << "<TD BGCOLOR=\"#FDFEFE\" ALIGN=\"LEFT\"><B>" << k << "</B></TD>"
                  << "<TD BGCOLOR=\"#FDFEFE\" ALIGN=\"LEFT\">" << v << "</TD>"
                  << "</TR>\n";
        };

        // Todos los campos del inodo
        row("i_uid",   to_string(inode.i_uid));
        row("i_gid",   to_string(inode.i_gid));
        row("i_size",  to_string(inode.i_size));
        row("i_atime", fmtTime(inode.i_atime));
        row("i_ctime", fmtTime(inode.i_ctime));
        row("i_mtime", fmtTime(inode.i_mtime));
        row("i_type",  string(1, inode.i_type));
        row("i_perm",  to_string(inode.i_perm));

        // Solo bloques usados
        for (int b = 0; b < 15; b++) {
            if (inode.i_block[b] == -1) continue;
            row("i_block[" + to_string(b) + "]", to_string(inode.i_block[b]));
        }

        nodes << "    </TABLE>>];\n\n";

        if (!prevNode.empty())
            edges << "  " << prevNode << " -> " << nodeId << ";\n";
        prevNode = nodeId;
    }

    ostringstream dot;
    dot << "digraph INODES {\n";
    dot << "  node [shape=plaintext fontname=\"Helvetica\"]\n";
    dot << "  rankdir=LR\n\n";
    dot << nodes.str();
    dot << edges.str();
    dot << "}\n";

    renderDot(dot.str(), path);
}
