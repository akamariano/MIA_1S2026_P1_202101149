#ifndef BLOCKS_H
#define BLOCKS_H

#include <cstring>

#pragma pack(push, 1)

// --------- CONTENT (Entrada de Carpeta) ---------
struct Content {
    char b_name[12];   // Nombre archivo/carpeta
    int b_inodo;       // Número de inodo

    Content() {
        memset(b_name, 0, sizeof(b_name));
        b_inodo = -1;
    }
};

// --------- BLOQUE DE CARPETA ---------
struct DirectoryBlock {
    Content b_content[4];  // 4 entradas

    DirectoryBlock() {
        // Inicializa automáticamente los 4 contenidos
        for(int i = 0; i < 4; i++) {
            b_content[i] = Content();
        }
    }
};

// --------- BLOQUE DE ARCHIVO ---------
struct FileBlock {
    char b_content[64];   // Contenido del archivo (64 bytes)

    FileBlock() {
        memset(b_content, 0, sizeof(b_content));
    }
};

// --------- BLOQUE DE APUNTADORES ---------
struct PointerBlock {
    int b_pointers[16];   // 16 apuntadores a bloques

    PointerBlock() {
        for(int i = 0; i < 16; i++) {
            b_pointers[i] = -1;  // -1 = no usado
        }
    }
};


#pragma pack(pop)

#endif
