#ifndef JOURNAL_MANAGER_H
#define JOURNAL_MANAGER_H

#include "SuperBlock.h"
#include "Journal.h"
#include "EXT2Utils.h"
#include <string>
#include <cstring>
#include <ctime>
#include <cstdio>
#include <vector>
#include <algorithm>
class JournalManager {
public:

    // Escribe una entrada en el journal (solo si es EXT3)
    static void write(FILE* disk, SuperBlock& sb, long long partStart,
                      const std::string& operation,
                      const std::string& path,
                      const std::string& content = "") {

        // Solo EXT3 tiene journal
        if (sb.s_filesystem_type != 3) return;

        int numJournals = sb.s_inodes_count;

        // Calcular inicio del journal
        // journal_start = partStart + sizeof(SuperBlock)
        long long journalStart = partStart + sizeof(SuperBlock);

        // Buscar el siguiente slot libre (j_count == 0)
        // o el slot con j_count más bajo para sobreescribir (circular)
        int targetSlot = -1;
        int minCount   = -1;

        for (int i = 0; i < numJournals; i++) {
            Journal j;
            long long pos = journalStart + (long long)i * sizeof(Journal);
            fseek(disk, pos, SEEK_SET);
            fread(&j, sizeof(Journal), 1, disk);

            if (j.j_count == 0) {
                targetSlot = i;
                break;
            }
            if (minCount == -1 || j.j_count < minCount) {
                minCount   = j.j_count;
                targetSlot = i;
            }
        }

        if (targetSlot == -1) return;

        // Leer el journal actual para obtener el count máximo
        int maxCount = 0;
        for (int i = 0; i < numJournals; i++) {
            Journal j;
            long long pos = journalStart + (long long)i * sizeof(Journal);
            fseek(disk, pos, SEEK_SET);
            fread(&j, sizeof(Journal), 1, disk);
            if (j.j_count > maxCount) maxCount = j.j_count;
        }

        // Crear nueva entrada
        Journal entry;
        memset(&entry, 0, sizeof(Journal));
        entry.j_count = maxCount + 1;

        strncpy(entry.j_content.i_operation, operation.c_str(), 9);
        strncpy(entry.j_content.i_path,      path.c_str(),      31);
        strncpy(entry.j_content.i_content,   content.c_str(),   63);
        entry.j_content.i_date = (float)time(nullptr);

        // Escribir en disco
        long long pos = journalStart + (long long)targetSlot * sizeof(Journal);
        fseek(disk, pos, SEEK_SET);
        fwrite(&entry, sizeof(Journal), 1, disk);
    }

    // Leer todos los journals de una partición
    static std::vector<Journal> readAll(FILE* disk, SuperBlock& sb,
                                         long long partStart) {
        std::vector<Journal> result;
        if (sb.s_filesystem_type != 3) return result;

        int numJournals    = sb.s_inodes_count;
        long long journalStart = partStart + sizeof(SuperBlock);

        for (int i = 0; i < numJournals; i++) {
            Journal j;
            long long pos = journalStart + (long long)i * sizeof(Journal);
            fseek(disk, pos, SEEK_SET);
            fread(&j, sizeof(Journal), 1, disk);
            if (j.j_count > 0) result.push_back(j);
        }

        // Ordenar por j_count
        std::sort(result.begin(), result.end(),
                  [](const Journal& a, const Journal& b){
                      return a.j_count < b.j_count;
                  });
        return result;
    }
};

#endif