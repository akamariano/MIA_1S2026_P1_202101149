#include "ext2_writer.h"
#include "../core/filesystem/EXT2Utils.h"
#include <iostream>
#include <cstring>
#include <ctime>

using namespace std;

// ============================================================
// PRIVADO: crear carpeta y registrarla en el padre
// ============================================================
int EXT2Writer::createDirectory(FILE* disk, SuperBlock& sb,
                                long long partStart,
                                int parentInode,
                                const string& name,
                                int uid, int gid) {
    // 1. Asignar inodo
    int newInodeNum = allocateInode(disk, sb, partStart);
    if (newInodeNum == -1) {
        cout << "ERROR: No hay inodos libres\n";
        return -1;
    }

    // 2. Asignar bloque para contenido de la carpeta
    int newBlockNum = allocateBlock(disk, sb, partStart);
    if (newBlockNum == -1) {
        cout << "ERROR: No hay bloques libres\n";
        return -1;
    }

    // 3. Crear inodo de carpeta
    Inode newInode;
    memset(&newInode, 0, sizeof(Inode));
    newInode.i_uid   = uid;
    newInode.i_gid   = gid;
    newInode.i_size  = sizeof(DirectoryBlock);
    newInode.i_type  = '0'; // carpeta
    newInode.i_perm  = 664;
    newInode.i_atime = newInode.i_ctime = newInode.i_mtime = time(nullptr);
    for (int i = 0; i < 15; i++) newInode.i_block[i] = -1;
    newInode.i_block[0] = newBlockNum;

    // 4. Crear bloque carpeta con . y ..
    DirectoryBlock db;
    memset(&db, 0, sizeof(DirectoryBlock));
    for (int i = 0; i < 4; i++) db.b_content[i].b_inodo = -1;

    strncpy(db.b_content[0].b_name, ".",  11);
    db.b_content[0].b_inodo = newInodeNum;

    strncpy(db.b_content[1].b_name, "..", 11);
    db.b_content[1].b_inodo = parentInode;

    db.b_content[2].b_inodo = -1;
    db.b_content[3].b_inodo = -1;

    // 5. Escribir en disco
    writeInode(disk, sb, newInodeNum, newInode);
    writeBlock(disk, sb, newBlockNum, &db);

    // 6. Registrar en el padre
    if (!addEntryToDirectory(disk, sb, partStart, parentInode, name, newInodeNum)) {
        cout << "ERROR: No se pudo agregar entrada en directorio padre\n";
        return -1;
    }

    return newInodeNum;
}

// ============================================================
// PRIVADO: escribir contenido string en bloques de archivo
// ============================================================
bool EXT2Writer::writeFileContent(FILE* disk, SuperBlock& sb,
                                   long long partStart,
                                   Inode& fileInode, int inodeNum,
                                   const string& content) {
    int remaining = content.size();
    int offset    = 0;
    int blockIdx  = 0; // índice en i_block[]

    while (remaining > 0 && blockIdx < 12) {
        int newBlock = allocateBlock(disk, sb, partStart);
        if (newBlock == -1) {
            cout << "ERROR: No hay bloques libres para contenido\n";
            return false;
        }

        FileBlock fb;
        memset(&fb, 0, sizeof(FileBlock));

        int writeSize = (remaining > 64) ? 64 : remaining;
        memcpy(fb.b_content, content.c_str() + offset, writeSize);

        writeBlock(disk, sb, newBlock, &fb);
        fileInode.i_block[blockIdx] = newBlock;

        offset    += writeSize;
        remaining -= writeSize;
        blockIdx++;
    }

    // Actualizar tamaño del inodo
    fileInode.i_size  = content.size();
    fileInode.i_mtime = time(nullptr);
    writeInode(disk, sb, inodeNum, fileInode);

    return true;
}

// ============================================================
// PÚBLICO: MKDIR
// ============================================================
bool EXT2Writer::mkdir(FILE* disk, SuperBlock& sb, long long partStart,
                       const string& path, bool createParents,
                       int uid, int gid) {
    vector<string> parts = splitPath(path);

    if (parts.empty()) {
        cout << "ERROR: Ruta inválida\n";
        return false;
    }

    int currentInode = 0; // root

    for (int i = 0; i < (int)parts.size(); i++) {
        int found = findInDirectory(disk, sb, currentInode, parts[i]);

        if (found != -1) {
            // Ya existe — verificar que sea carpeta
            Inode existing;
            readInode(disk, sb, found, existing);
            if (existing.i_type != '0') {
                cout << "ERROR: '" << parts[i] << "' ya existe y no es carpeta\n";
                return false;
            }
            currentInode = found;
            continue;
        }

        // No existe
        bool isLast = (i == (int)parts.size() - 1);

        if (!isLast && !createParents) {
            cout << "ERROR: La carpeta padre '" << parts[i] << "' no existe\n";
            cout << "       Use -p para crear carpetas padres\n";
            return false;
        }

        // Crear la carpeta
        int newInode = createDirectory(disk, sb, partStart,
                                       currentInode, parts[i],
                                       uid, gid);
        if (newInode == -1) return false;

        currentInode = newInode;
    }

    cout << "OK: Carpeta '" << path << "' creada correctamente\n";
    return true;
}

// ============================================================
// PÚBLICO: MKFILE
// ============================================================
bool EXT2Writer::mkfile(FILE* disk, SuperBlock& sb, long long partStart,
                         const string& path, bool createParents,
                         int size, const string& content,
                         int uid, int gid) {
    vector<string> parts = splitPath(path);

    if (parts.empty()) {
        cout << "ERROR: Ruta inválida\n";
        return false;
    }

    string filename = parts.back();
    parts.pop_back(); // separar nombre del archivo de las carpetas padre

    // Navegar hasta la carpeta padre
    int currentInode = 0;
    for (int i = 0; i < (int)parts.size(); i++) {
        int found = findInDirectory(disk, sb, currentInode, parts[i]);

        if (found == -1) {
            if (!createParents) {
                cout << "ERROR: La carpeta '" << parts[i] << "' no existe\n";
                cout << "       Use -r para crear carpetas padres\n";
                return false;
            }
            // Crear carpeta padre
            int newInode = createDirectory(disk, sb, partStart,
                                           currentInode, parts[i],
                                           uid, gid);
            if (newInode == -1) return false;
            currentInode = newInode;
        } else {
            Inode existing;
            readInode(disk, sb, found, existing);
            if (existing.i_type != '0') {
                cout << "ERROR: '" << parts[i] << "' no es una carpeta\n";
                return false;
            }
            currentInode = found;
        }
    }

    // Verificar si el archivo ya existe
    int existingFile = findInDirectory(disk, sb, currentInode, filename);
    if (existingFile != -1) {
        cout << "ADVERTENCIA: El archivo '" << filename << "' ya existe. ¿Sobreescribir? (Y/N): ";
        char resp;
        cin >> resp;
        cin.ignore();
        if (toupper(resp) != 'Y') {
            cout << "Operación cancelada\n";
            return false;
        }
        // Sobreescribir: reutilizar inodo existente
        Inode existing;
        readInode(disk, sb, existingFile, existing);
        // Liberar bloques anteriores
        for (int b = 0; b < 12; b++) {
            if (existing.i_block[b] != -1) {
                writeBitmapBlock(disk, sb, existing.i_block[b], 0);
                sb.s_free_blocks_count++;
                existing.i_block[b] = -1;
            }
        }
        // Escribir nuevo contenido
        string finalContent = content;
        if (finalContent.empty() && size > 0) {
            for (int i = 0; i < size; i++)
                finalContent += (char)('0' + (i % 10));
        }
        writeFileContent(disk, sb, partStart, existing, existingFile, finalContent);
        writeSuperBlock(disk, partStart, sb);
        cout << "OK: Archivo '" << filename << "' sobreescrito\n";
        return true;
    }

    // Crear nuevo inodo de archivo
    int newInodeNum = allocateInode(disk, sb, partStart);
    if (newInodeNum == -1) {
        cout << "ERROR: No hay inodos libres\n";
        return false;
    }

    Inode newInode;
    memset(&newInode, 0, sizeof(Inode));
    newInode.i_uid   = uid;
    newInode.i_gid   = gid;
    newInode.i_size  = 0;
    newInode.i_type  = '1'; // archivo
    newInode.i_perm  = 664;
    newInode.i_atime = newInode.i_ctime = newInode.i_mtime = time(nullptr);
    for (int i = 0; i < 15; i++) newInode.i_block[i] = -1;

    writeInode(disk, sb, newInodeNum, newInode);

    // Determinar contenido final
    string finalContent = content;
    if (finalContent.empty() && size > 0) {
        for (int i = 0; i < size; i++)
            finalContent += (char)('0' + (i % 10));
    }

    // Escribir contenido si hay
        if (!finalContent.empty()) {
            if (!writeFileContent(disk, sb, partStart, newInode, newInodeNum, finalContent)) {
                return false;
            }
        } else {
            // Sin contenido, solo actualizar tamaño
            newInode.i_size = 0;
            writeInode(disk, sb, newInodeNum, newInode);
        }

    // Registrar en carpeta padre
    if (!addEntryToDirectory(disk, sb, partStart, currentInode, filename, newInodeNum)) {
        cout << "ERROR: No se pudo registrar el archivo en la carpeta padre\n";
        return false;
    }

    writeSuperBlock(disk, partStart, sb);
    cout << "OK: Archivo '" << path << "' creado correctamente\n";
    return true;
}

// ============================================================
// PÚBLICO: CREAR USERS.TXT INICIAL (llamado desde mkfs)
// ============================================================
bool EXT2Writer::createUsersFile(FILE* disk, SuperBlock& sb,
                                  long long partStart) {
    // Contenido inicial
    string content = "1,G,root\n1,U,root,root,123\n";

    // Crear carpeta /users si no existe (en root ya existe /)
    // El archivo va directo en root: /users.txt
    return mkfile(disk, sb, partStart,
                  "/users.txt",
                  false,        // no crear padres
                  0,            // size 0 (usamos content)
                  content,
                  1,            // uid root
                  1);           // gid root
}

// ============================================================
// PÚBLICO: REESCRIBIR USERS.TXT
// ============================================================
bool EXT2Writer::writeUsersFile(FILE* disk, SuperBlock& sb,
                                 long long partStart,
                                 const string& newContent) {
    int inodeNum = resolvePath(disk, sb, "/users.txt");
    if (inodeNum == -1) {
        cout << "ERROR: users.txt no encontrado\n";
        return false;
    }

    Inode fileInode;
    readInode(disk, sb, inodeNum, fileInode);

    // Liberar bloques anteriores
    for (int b = 0; b < 12; b++) {
        if (fileInode.i_block[b] != -1) {
            writeBitmapBlock(disk, sb, fileInode.i_block[b], 0);
            sb.s_free_blocks_count++;
            fileInode.i_block[b] = -1;
        }
    }

    return writeFileContent(disk, sb, partStart, fileInode, inodeNum, newContent);
}

// ============================================================
// PÚBLICO: LEER USERS.TXT COMPLETO
// ============================================================
string EXT2Writer::readUsersFile(FILE* disk, SuperBlock& sb,
                                  long long partStart) {
    int inodeNum = resolvePath(disk, sb, "/users.txt");
    if (inodeNum == -1) return "";

    Inode fileInode;
    readInode(disk, sb, inodeNum, fileInode);

    string result;
    for (int b = 0; b < 12; b++) {
        if (fileInode.i_block[b] == -1) break;
        FileBlock fb;
        memset(&fb, 0, sizeof(FileBlock));

        readBlock(disk, sb, fileInode.i_block[b], &fb);
        // Solo leer hasta el tamaño real del archivo
        int toRead = min((int)sizeof(fb.b_content),
                         fileInode.i_size - (int)result.size());
        if (toRead <= 0) break;
        result.append(fb.b_content, toRead);
    }

    return result;
}