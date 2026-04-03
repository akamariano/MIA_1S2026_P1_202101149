#include "journaling_cmd.h"
#include "../mount/mount_manager.h"
#include "../filesystem/session_manager.h"
#include "../filesystem/journal_manager.h"
#include "../core/filesystem/EXT2Utils.h"
#include "../core/filesystem/SuperBlock.h"
#include "../core/filesystem/Journal.h"
#include <iostream>
#include <cstring>
#include <ctime>
#include <vector>
#include <algorithm>
using namespace std;

void JournalingCmd::execute(const string& id) {

    MountedPartition* part = MountManager::getMountedById(id);
    if (!part) {
        cout << "ERROR: ID '" << id << "' no está montado\n"; return;
    }

    FILE* disk = fopen(part->path.c_str(), "rb");
    if (!disk) {
        cout << "ERROR: No se pudo abrir el disco\n"; return;
    }

    SuperBlock sb;
    readSuperBlock(disk, part->start, sb);

    if (sb.s_filesystem_type != 3) {
        cout << "ERROR: La partición '" << id << "' no es EXT3\n";
        fclose(disk); return;
    }

    vector<Journal> journals = JournalManager::readAll(disk, sb, part->start);
    fclose(disk);

    if (journals.empty()) {
        cout << "No hay entradas en el journal de '" << id << "'\n";
        return;
    }

    cout << "========================================\n";
    cout << "  JOURNALING - Partición: " << id << "\n";
    cout << "========================================\n";

    for (auto& j : journals) {
        // Convertir float timestamp a fecha legible
        time_t t = (time_t)j.j_content.i_date;
        char dateBuf[32];
        struct tm* ti = localtime(&t);
        strftime(dateBuf, sizeof(dateBuf), "%d/%m/%Y %H:%M:%S", ti);

        // Limpiar strings (pueden tener basura después del \0)
        string op(j.j_content.i_operation,
                  strnlen(j.j_content.i_operation, 10));
        string path(j.j_content.i_path,
                    strnlen(j.j_content.i_path, 32));
        string content(j.j_content.i_content,
                       strnlen(j.j_content.i_content, 64));

        cout << "----------------------------------------\n";
        cout << "  #" << j.j_count << "\n";
        cout << "  Operacion : " << op      << "\n";
        cout << "  Ruta      : " << path    << "\n";
        if (!content.empty())
            cout << "  Contenido : " << content << "\n";
        cout << "  Fecha     : " << dateBuf << "\n";
    }

    cout << "========================================\n";
    cout << "Total: " << journals.size() << " operacion(es)\n";
}