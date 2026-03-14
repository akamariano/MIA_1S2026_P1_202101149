#include "rep_cmd.h"
#include "../reports/report_mbr.h"
#include "../reports/report_disk.h"
#include "../reports/report_sb.h"
#include "../reports/report_inode.h"
#include "../reports/report_block.h"
#include "../reports/report_tree.h"
#include "../reports/report_bm.h"
#include "../reports/report_file.h"
#include "../reports/report_ls.h"
#include "../mount/mount_manager.h"
#include <iostream>
#include <filesystem>
#include <algorithm>
using namespace std;

void RepCmd::execute(const string& name, const string& path,
                     const string& id,  const string& pathFileLs) {

    MountedPartition* part = MountManager::getMountedById(id);
    if (!part) {
        cout << "ERROR: ID '" << id << "' no está montado\n"; return;
    }

    // Crear carpeta destino si no existe
    filesystem::path outPath(path);
    filesystem::create_directories(outPath.parent_path());

    FILE* disk = fopen(part->path.c_str(), "rb");
    if (!disk) {
        cout << "ERROR: No se pudo abrir el disco\n"; return;
    }

    if      (name == "mbr")      ReportMbr::generate(disk, part, path);
    else if (name == "disk")     ReportDisk::generate(disk, part, path);
    else if (name == "sb")       ReportSb::generate(disk, part, path);
    else if (name == "inode")    ReportInode::generate(disk, part, path);
    else if (name == "block")    ReportBlock::generate(disk, part, path);
    else if (name == "tree")     ReportTree::generate(disk, part, path);
    else if (name == "bm_inode") ReportBm::generateInodeBitmap(disk, part, path);
    else if (name == "bm_block") ReportBm::generateBlockBitmap(disk, part, path);
    else if (name == "file") {
        // Detectar extensión para decidir formato
        string ext = path;
        transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        string extension = "";
        size_t dot = ext.rfind('.');
        if (dot != string::npos) extension = ext.substr(dot);
        ReportFile::generate(disk, part, path, pathFileLs, extension);
    } // si es .txt hace un text y si es .jpg lo renderizara por un graphviz
    else if (name == "ls")       ReportLs::generate(disk, part, path, pathFileLs);
    else cout << "ERROR: Reporte '" << name << "' no reconocido\n";

    fclose(disk);
}