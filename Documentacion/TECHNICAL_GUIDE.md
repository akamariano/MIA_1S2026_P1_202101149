# Manual Técnico - EXTREAMFS
| Nombre | Carnet|
|----------|-------|
| Mariano Roberto Rac Noguera| 202101149 |


---

## Tabla de Contenidos
1. [Descripción General](#descripción-general)
2. [Arquitectura del Sistema](#arquitectura-del-sistema)
3. [Estructura de Carpetas](#estructura-de-carpetas)
4. [Tecnologías Utilizadas](#tecnologías-utilizadas)
5. [Estructuras de Datos](#estructuras-de-datos)
6. [Comandos Implementados](#comandos-implementados)
7. [Diagramas de Clase](#diagramas-de-clase)
8. [Flujo de Ejecución](#flujo-de-ejecución)
9. [Compilación y Construcción](#compilación-y-construcción)
10. [Detalles de Implementación](#detalles-de-implementación)

---

## Descripción General

EXTREAMFS es un **simulador de sistema de archivos EXT2** que implementa:

- **Gestión de Discos**: Creación, particionamiento y eliminación de discos virtuales
- **Sistemas de Archivos**: Formato EXT2 completo con soporte para archivos y directorios
- **Control de Acceso**: Sistema de usuarios, grupos y permisos POSIX
- **Reportes**: Visualización gráfica de la estructura interna del sistema de archivos
- **API HTTP**: Interfaz servidor para operaciones remotas
- **Interfaz Web**: Frontend interactivo construido con Next.js

---

## Arquitectura del Sistema

### Arquitectura de Capas

```
┌─────────────────────────────────┐
│    Frontend (Next.js/React)     │
│  - Interfaz de Usuario          │
│  - Componentes React            │
│  - Estado Global con Hooks      │
└─────────────┬───────────────────┘
              │ HTTP REST API
┌─────────────▼───────────────────┐
│  Backend (C++ / HTTP Server)    │
│  - Procesamiento de Comandos    │
│  - Lógica de Negocio            │
│  - Gestión de Discos            │
└─────────────┬───────────────────┘
              │
┌─────────────▼───────────────────┐
│      Gestor de Montaje          │
│  - Gestión de Particiones       │
│  - Puntos de Montaje            │
└─────────────┬───────────────────┘
              │
┌─────────────▼───────────────────┐
│     Sistema de Archivos EXT2    │
│  - Lectura/Escritura de Bloques │
│  - Gestión de Inodos            │
│  - Tablas de Usuarios/Grupos    │
└─────────────┬───────────────────┘
              │
┌─────────────▼───────────────────┐
│    Gestor de Discos Físicos     │
│  - MBR (Master Boot Record)     │
│  - EBR (Extended Boot Record)   │
│  - Asignación de Espacio        │
└─────────────────────────────────┘
```

### Modelo de Datos

```
Disco (disco.dsk)
│
├── MBR (512 bytes)
│   ├── Tabla de Particiones (4 primarias)
│   └── Firma (0xAA55)
│
├── Partición Primaria/Lógica
│   ├── SuperBloque EXT2
│   ├── Tabla de Inodos
│   ├── Mapa de Bits (Bloques)
│   ├── Mapa de Bits (Inodos)
│   ├── Bloques de Datos
│   └── Tabla de Usuarios/Grupos
│
└── EBR (para particiones extendidas)
```

---

## Estructura de Carpetas

```
MIA_1S2026_P1_202101149/
├── frontend/                          # Aplicación completa
│   ├── backend/                       # Backend C++
│   │   ├── core/                      # Lógica principal
│   │   │   ├── commands/              # Implementación de comandos
│   │   │   │   ├── mkdisk.{h,cpp}    # Crear disco
│   │   │   │   ├── fdisk.{h,cpp}     # Particionar disco
│   │   │   │   ├── mkfs.{h,cpp}      # Crear sistema de archivos
│   │   │   │   ├── mount.{h,cpp}     # Montar partición
│   │   │   │   ├── mkdir_cmd.{h,cpp} # Crear directorio
│   │   │   │   ├── mkfile_cmd.{h,cpp}# Crear archivo
│   │   │   │   ├── cat_cmd.{h,cpp}   # Leer archivo
│   │   │   │   ├── mkusr.{h,cpp}     # Crear usuario
│   │   │   │   ├── mkgrp.{h,cpp}     # Crear grupo
│   │   │   │   ├── chgrp.{h,cpp}     # Cambiar grupo
│   │   │   │   ├── rmusr.{h,cpp}     # Eliminar usuario
│   │   │   │   ├── rmgrp.{h,cpp}     # Eliminar grupo
│   │   │   │   ├── rmdisk.{h,cpp}    # Eliminar disco
│   │   │   │   └── rep_cmd.{h,cpp}   # Generar reportes
│   │   │   │
│   │   │   ├── disk/                 # Gestión de discos
│   │   │   │   ├── disk_creator.cpp  # Crear archivos de disco
│   │   │   │   ├── mbr.h             # Estructura MBR
│   │   │   │   └── ebr.h             # Estructura EBR
│   │   │   │
│   │   │   ├── filesystem/           # Sistema de archivos
│   │   │   │   ├── ext2_reader.{h,cpp}    # Lectura de EXT2
│   │   │   │   ├── ext2_writer.{h,cpp}    # Escritura de EXT2
│   │   │   │   ├── SuperBlock.{h,cpp}     # Superbloque
│   │   │   │   ├── Inode.h                # Estructura de Inode
│   │   │   │   ├── Blocks.h               # Estructuras de Bloques
│   │   │   │   ├── login.{h,cpp}         # Autenticación
│   │   │   │   ├── logout.{h,cpp}        # Cierre de sesión
│   │   │   │   ├── session_manager.{h,cpp} # Gestión de sesiones
│   │   │   │   ├── permissions.h         # Sistema de permisos
│   │   │   │   ├── users_manager.h       # Gestión de usuarios
│   │   │   │   └── EXT2Utils.h           # Utilidades EXT2
│   │   │   │
│   │   │   ├── mount/                # Gestión de montaje
│   │   │   │   └── mount_manager.{h,cpp}
│   │   │   │
│   │   │   ├── reports/             # Generadores de reportes
│   │   │   │   ├── report_mbr.{h,cpp}
│   │   │   │   ├── report_sb.{h,cpp}
│   │   │   │   ├── report_bm.{h,cpp}
│   │   │   │   ├── report_inode.{h,cpp}
│   │   │   │   ├── report_block.{h,cpp}
│   │   │   │   ├── report_disk.{h,cpp}
│   │   │   │   ├── report_ls.{h,cpp}
│   │   │   │   ├── report_tree.{h,cpp}
│   │   │   │   ├── report_file.{h,cpp}
│   │   │   │   └── report_utils.h
│   │   │   │
│   │   │   ├── server/              # Servidor HTTP
│   │   │   │   └── server.{h,cpp}
│   │   │   │
│   │   │   └── utils/               # Utilidades generales
│   │   │
│   │   ├── include/                 # Headers de terceros
│   │   │   ├── httplib.h            # Librería HTTP
│   │   │   └── main.cpp             # Punto de entrada
│   │   │
│   │   ├── build/                   # Directorio de compilación
│   │   │   ├── extreamfs            # Ejecutable (Modo consola)
│   │   │   └── extreamfs_debug      # Ejecutable (Debug)
│   │   │
│   │   └── CMakeLists.txt           # Configuración CMake
│   │
│   ├── components/                  # Componentes React
│   │   ├── CommandPanel.tsx         # Panel de entrada de comandos
│   │   ├── Terminal.tsx             # Terminal interactiva
│   │   ├── TerminalInput.tsx        # Input de terminal
│   │   ├── TerminalOutput.tsx       # Output de terminal
│   │   ├── FileExplorer.tsx         # Explorador de archivos
│   │   └── BlockViewer.tsx          # Visor de bloques
│   │
│   ├── app/                         # Aplicación Next.js
│   │   ├── page.tsx                 # Página principal
│   │   ├── layout.tsx               # Layout global
│   │   └── globals.css              # Estilos globales
│   │
│   ├── services/                    # Servicios
│   │   └── api.ts                   # Cliente HTTP para backend
│   │
│   ├── types/                       # Tipos TypeScript
│   │   └── filesystem.ts            # Tipos del sistema de archivos
│   │
│   ├── hooks/                       # React Hooks
│   │   └── useFilesystem.ts         # Hook para gestión de FS
│   │
│   ├── package.json                 # Dependencias Node.js
│   ├── tsconfig.json                # Configuración TypeScript
│   ├── next.config.ts               # Configuración Next.js
│   ├── postcss.config.mjs           # Configuración PostCSS
│   └── eslint.config.mjs            # Configuración ESLint
│
├── Documentacion/                    # Documentación
│   └── images/                      # Imágenes y reportes
│       ├── CONSOLAS.png
│       ├── INTERFAZ.png
│       ├── disco1_block.jpg
│       ├── disco1_inode.jpg
│       ├── disco1_ls_docs.jpg
│       ├── disco1_mbr.jpg
│       ├── disco1_sb.jpg
│       └── disco1_tree.jpg
│
├── USER_GUIDE.md                    # Manual de usuario
├── TECHNICAL_GUIDE.md               # Manual técnico
├── README.md                        # Descripción del proyecto
└── test.smia                        # Archivo de prueba
```

---

## Tecnologías Utilizadas

### Backend
| Tecnología | Versión | Propósito |
|-----------|---------|----------|
| C++ | C++11/14 | Lenguaje principal |
| CMake | 3.22+ | Sistema de compilación |
| cpp-httplib | 0.11+ | Servidor HTTP |
| STL | Estándar | Estructuras de datos |

### Frontend
| Tecnología | Versión | Propósito |
|-----------|---------|----------|
| Next.js | 14+ | Framework web |
| React | 18+ | Librería UI |
| TypeScript | 5+ | Tipado de JavaScript |
| Tailwind CSS | 3+ | Framework CSS |
| Node.js | 16+ | Runtime JavaScript |

### Herramientas
| Herramienta | Propósito |
|-----------|----------|
| CMake | Compilación C++ |
| npm | Gestor de paquetes |
| Git | Control de versiones |
| GCC/Clang | Compiladores C++ |

---

## Estructuras de Datos

### 1. Master Boot Record (MBR)

```cpp
struct Partition {
    char status;              // 0x00 o 0x80 (inactiva/activa)
    char first_head;          // CHS de inicio
    char first_sector;
    char first_cylinder;
    char id;                  // Tipo de partición
    char last_head;           // CHS de fin
    char last_sector;
    char last_cylinder;
    int start;                // LBA de inicio
    int size;                 // Tamaño de la partición
};

struct MBR {
    char padding[446];        // Código de arranque
    Partition particiones[4]; // Tabla de particiones (4 entradas)
    char firma[2];            // 0xAA 0x55 (firma del MBR)
};
```

### 2. Extended Boot Record (EBR)

```cpp
struct EBR {
    char nothing[446];
    Partition partition;      // Partición lógica actual
    Partition extended;       // Referencia a siguiente EBR
    char nothing2[60];
    char firma[2];            // Firma 0xAA 0x55
};
```

### 3. SuperBloque EXT2

```cpp
struct SuperBlock {
    unsigned int total_inodes;         // Total de inodos
    unsigned int total_blocks;         // Total de bloques
    unsigned int reserved_blocks;      // Bloques reservados
    unsigned int free_blocks;          // Bloques libres
    unsigned int free_inodes;          // Inodos libres
    unsigned int block_size;           // Tamaño de bloque
    unsigned int inode_size;           // Tamaño de inodo
    unsigned int blocks_per_group;     // Bloques por grupo
    unsigned int inodes_per_group;     // Inodos por grupo
    unsigned int creation_time;        // Tiempo de creación
    unsigned int last_write_time;      // Último acceso
    unsigned int fsck_count;           // Contador FSCK
    unsigned int max_mount_count;      // Montajes máximos
    char uuid[16];                     // Identificador único
    char volume_name[16];              // Nombre de volumen
};
```

### 4. Inode EXT2

```cpp
struct Inode {
    unsigned short mode;               // Tipo y permisos
    unsigned short uid;                // ID de propietario
    unsigned int size;                 // Tamaño del archivo
    unsigned int access_time;          // Tiempo de acceso
    unsigned int creation_time;        // Tiempo de creación
    unsigned int modification_time;    // Tiempo de modificación
    unsigned int deletion_time;        // Tiempo de eliminación
    unsigned short gid;                // ID de grupo
    unsigned short link_count;         // Número de enlaces
    unsigned int disk_sectors;         // Sectores en disco
    unsigned int flags;                // Banderas
    unsigned int os_specific_1;        // OS específico
    unsigned int block_pointers[15];   // Punteros a bloques
    unsigned int generation_number;    // Número de generación
    unsigned int file_acl;             // ACL de archivo
    unsigned int dir_acl_or_size_high; // ACL de directorio
    unsigned int fragment_block_addr;  // Dirección de bloque
    unsigned char os_specific_2[12];   // OS específico
};
```

### 5. Entrada de Directorio

```cpp
struct Content {
    unsigned int inode_number;         // Número de inode
    unsigned short total_size;         // Tamaño total de entrada
    unsigned char name_length;         // Longitud del nombre
    unsigned char type_indicator;      // Tipo de entrada
    char name[256];                    // Nombre del archivo
};
```

### 6. Bloque de Directorio

```cpp
struct DirectoryBlock {
    Content contents[1024];            // Entradas de directorio
};
```

### 7. Bloque de Archivo

```cpp
struct FileBlock {
    char data[4096];                   // Datos del archivo
};
```

### 8. Entrada de Usuario

```cpp
struct UserEntry {
    int user_id;                       // ID único del usuario
    char username[50];                 // Nombre de usuario
    char password[256];                // Hash de contraseña
    unsigned int creation_date;        // Fecha de creación
    char description[150];             // Descripción
};
```

### 9. Entrada de Grupo

```cpp
struct GroupEntry {
    int group_id;                      // ID único del grupo
    char group_name[50];               // Nombre del grupo
    unsigned int creation_date;        // Fecha de creación
    char description[150];             // Descripción
};
```

---

## Comandos Implementados

### 1. mkdisk - Crear Disco
```
Sintaxis: mkdisk -size=<número> -unit=<K|M|G> -fit=<FF|BF|WF> -path=<ruta>

Función:
- Crea un archivo de disco virtual con tamaño especificado
- Inicializa el MBR en el sector 0
- Asigna espacio en disco según fitSize

Parámetros:
- size: Tamaño del disco (entero)
- unit: K=Kilobytes, M=Megabytes, G=Gigabytes
- fit: FF=First Fit, BF=Best Fit, WF=Worst Fit
- path: Ruta completa del archivo a crear
```

### 2. fdisk - Particionar Disco
```
Sintaxis: fdisk -size=<número> -unit=<K|M|G> -type=<P|E|L> -fit=<FF|BF|WF> 
              -delete=<0|1> -name=<nombre> -path=<ruta>

Función:
- Crea o elimina particiones en el disco
- Soporta particiones primarias, extendidas y lógicas
- Maneja MBR y EBR automáticamente

Parámetros:
- size: Tamaño de la partición
- type: P=Primaria, E=Extendida, L=Lógica
- delete: 0=crear, 1=eliminar
- name: Identificador de partición
- fit: Estrategia de asignación
```

### 3. mkfs - Crear Sistema de Archivos
```
Sintaxis: mkfs -type=<ext2> -fs=<ext2> -id=<partition_id> -path=<ruta>

Función:
- Formatea una partición con EXT2
- Crea SuperBloque, inodos, y mapas de bits
- Inicializa tabla de usuarios (root)

Parámetros:
- type: Tipo de formato (ext2)
- fs: Sistema de archivos (ext2)
- id: ID de partición a formatear
- path: Ruta del disco
```

### 4. mount - Montar Partición
```
Sintaxis: mount -path=<ruta> -name=<nombre>

Función:
- Registra una partición como montada
- Habilita operaciones de lectura/escritura
- Mantiene tabla de particiones montadas

Parámetros:
- path: Ruta del disco
- name: Nombre de punto de montaje
```

### 5. mkdir - Crear Directorio
```
Sintaxis: mkdir -name=<ruta> -path=<disco>

Función:
- Crea un nuevo directorio en el sistema de archivos
- Asigna inode y bloque de directorio
- Actualiza directorio padre

Parámetros:
- name: Ruta del nuevo directorio
- path: Archivo de disco
```

### 6. mkfile - Crear Archivo
```
Sintaxis: mkfile -name=<ruta> -size=<bytes> -path=<disco>

Función:
- Crea un nuevo archivo regular
- Asigna inode y bloques de datos
- Inicializa permiso rwxr-xr-x

Parámetros:
- name: Ruta del nuevo archivo
- size: Tamaño en bytes
- path: Archivo de disco
```

### 7. cat - Leer Archivo
```
Sintaxis: cat -path=<disco> -file=<ruta>

Función:
- Lee contenido de archivo
- Muestra en consola o archivo
- Verifica permisos de lectura

Parámetros:
- path: Archivo de disco
- file: Ruta del archivo a leer
```

### 8. login - Autenticación
```
Sintaxis: login -user=<usuario> -pass=<contraseña> -id=<partition_id>

Función:
- Valida credenciales de usuario
- Inicia sesión en partición
- Almacena contexto de usuario actual

Parámetros:
- user: Nombre de usuario
- pass: Contraseña
- id: ID de partición
```

### 9. logout - Cerrar Sesión
```
Sintaxis: logout

Función:
- Cierra sesión actual
- Limpia contexto de usuario
- Desmonta sistema de archivos lógicamente
```

### 10. mkusr - Crear Usuario
```
Sintaxis: mkusr -user=<nombre> -pass=<contraseña> -id=<partition_id>

Función:
- Crea nuevas entradas de usuario
- Asigna UID automático
- Almacena hash de contraseña

Parámetros:
- user: Nombre de usuario
- pass: Contraseña
- id: Partición donde crear usuario
```

### 11. mkgrp - Crear Grupo
```
Sintaxis: mkgrp -name=<nombre> -id=<partition_id>

Función:
- Crea nuevas entradas de grupo
- Asigna GID automático
- Registra grupo en tabla de usuarios

Parámetros:
- name: Nombre del grupo
- id: Partición donde crear grupo
```

### 12. chgrp - Cambiar Grupo de Usuario
```
Sintaxis: chgrp -user=<usuario> -group=<grupo> -id=<partition_id>

Función:
- Asigna usuario a grupo
- Actualiza tabla de usuarios
- Sincroniza cambios en disco

Parámetros:
- user: Nombre de usuario
- group: Nombre de grupo
- id: Partición
```

### 13. rmusr - Eliminar Usuario
```
Sintaxis: rmusr -user=<usuario> -id=<partition_id>

Función:
- Elimina entrada de usuario
- Reasigna archivos a root
- Actualiza tabla de usuarios

Parámetros:
- user: Usuario a eliminar
- id: Partición
```

### 14. rmgrp - Eliminar Grupo
```
Sintaxis: rmgrp -name=<nombre> -id=<partition_id>

Función:
- Elimina entrada de grupo
- Desasigna usuarios del grupo
- Limpia referencias

Parámetros:
- name: Nombre de grupo
- id: Partición
```

### 15. rmdisk - Eliminar Disco
```
Sintaxis: rmdisk -path=<ruta>

Función:
- Elimina archivo de disco
- Desmonta particiones
- Limpia tabla de montaje

Parámetros:
- path: Ruta del archivo de disco
```

### 16. rep - Generar Reportes
```
Sintaxis: rep -name=<tipo> -path=<disco> -id=<partition_id>

Tipos de reportes:
- mbr: Tabla de particiones del MBR
- sb: Información del SuperBloque
- bm: Mapa de bits de bloques
- bit: Mapa de bits de inodos
- disk: Información general del disco
- tree: Árbol de directorios
- ls: Listado de archivos
- file: Información de archivo específico

Función:
- Genera reportes en formato GraphViz
- Visualiza estructura interna
- Exporta a PNG o JPG
```

---

## Diagramas de Clase

### Arquitectura de Clases del Sistema

```
┌──────────────────────────────────┐
│       MountManager               │
├──────────────────────────────────┤
│ - mountedPartitions[]            │
│ - activeSession                  │
├──────────────────────────────────┤
│ + mount()                        │
│ + unmount()                      │
│ + getActiveMountPoint()          │
└────────────┬─────────────────────┘
             │ usa
             ▼
┌──────────────────────────────────┐
│      EXT2Reader/Writer           │
├──────────────────────────────────┤
│ - diskFile                       │
│ - superBlock                     │
│ - inodeTable[]                   │
├──────────────────────────────────┤
│ + readInode()                    │
│ + writeInode()                   │
│ + readBlock()                    │
│ + writeBlock()                   │
│ + findFreeInode()               │
│ + findFreeBlock()               │
└────────────┬─────────────────────┘
             │ usa
             ▼
┌──────────────────────────────────┐
│        DiskCreator               │
├──────────────────────────────────┤
│ - diskPath                       │
│ - diskSize                       │
├──────────────────────────────────┤
│ + createDisk()                   │
│ + writeMBR()                     │
│ + writeEBR()                     │
└──────────────────────────────────┘

┌──────────────────────────────────┐
│         Login/Logout             │
├──────────────────────────────────┤
│ - currentUser                    │
│ - currentGroup                   │
├──────────────────────────────────┤
│ + authenticate()                 │
│ + logout()                       │
│ + getCurrentUser()               │
└──────────────────────────────────┘

┌──────────────────────────────────┐
│      SessionManager              │
├──────────────────────────────────┤
│ - sessions[]                     │
│ - activeUserID                   │
├──────────────────────────────────┤
│ + createSession()                │
│ + getSession()                   │
│ + validateSession()              │
└──────────────────────────────────┘

┌──────────────────────────────────┐
│       Permissions                │
├──────────────────────────────────┤
│ - mode: unsigned short            │
├──────────────────────────────────┤
│ + hasPermission()                │
│ + setPermission()                │
│ + checkOwnership()               │
└──────────────────────────────────┘
```

### Clases de Comandos

```
Comando Base (Interfaz)
    │
    ├─────────────────────────────────┬──────────────────────┐
    │                                  │                       │
    
┌────────────────┐  ┌───────────────┐  ┌──────────────────┐
│   MkDisk       │  │   FDisk       │  │    Mkfs          │
├────────────────┤  ├───────────────┤  ├──────────────────┤
│ + execute()    │  │ + execute()   │  │ + execute()      │
│ + validate()   │  │ + validate()  │  │ + validate()     │
└────────────────┘  └───────────────┘  └──────────────────┘

┌────────────────┐  ┌───────────────┐  ┌──────────────────┐
│   MkFile       │  │   MkDir       │  │    Cat           │
├────────────────┤  ├───────────────┤  ├──────────────────┤
│ + execute()    │  │ + execute()   │  │ + execute()      │
│ + validate()   │  │ + validate()  │  │ + validate()     │
└────────────────┘  └───────────────┘  └──────────────────┘

┌────────────────┐  ┌───────────────┐  ┌──────────────────┐
│   MkUsr        │  │   MkGrp       │  │    ChGrp         │
├────────────────┤  ├───────────────┤  ├──────────────────┤
│ + execute()    │  │ + execute()   │  │ + execute()      │
│ + validate()   │  │ + validate()  │  │ + validate()     │
└────────────────┘  └───────────────┘  └──────────────────┘

┌────────────────┐  ┌───────────────┐  ┌──────────────────┐
│   RepCmd       │  │   Mount       │  │    RmDisk        │
├────────────────┤  ├───────────────┤  ├──────────────────┤
│ + execute()    │  │ + execute()   │  │ + execute()      │
│ + validate()   │  │ + validate()  │  │ + validate()     │
└────────────────┘  └───────────────┘  └──────────────────┘
```

---

## Flujo de Ejecución

### Creación de Disco y Archivo

```
Usuario: mkdisk -size=100 -unit=M -path=/root/disco.dsk
    │
    ▼
main.cpp detecta comando
    │
    ▼
MkDisk::execute() es llamado
    │
    ├─ Valida parámetros
    ├─ Calcula tamaño total en bytes
    ├─ Crea archivo en disco
    └─ Inicializa MBR con sectores vacíos
    │
    ▼
Disco creado exitosamente
```

### Particionamiento

```
Usuario: fdisk -size=50 -unit=M -type=P -name=p1 -path=/root/disco.dsk
    │
    ▼
FDisk::execute() es llamado
    │
    ├─ Lee MBR existente
    ├─ Busca entrada libre en tabla de particiones
    ├─ Valida espacio disponible
    ├─ Crea nueva entrada de partición
    ├─ Actualiza tabla de particiones del MBR
    └─ Escribe MBR modificado en disco
    │
    ▼
Partición creada exitosamente
```

### Formateo y Login

```
Usuario: mkfs -type=ext2 -fs=ext2 -id=p1 -path=/root/disco.dsk
    │
    ▼
Mkfs::execute() es llamado
    │
    ├─ Lee partición desde disco
    ├─ Crea SuperBloque EXT2
    ├─ Inicializa tabla de inodos
    ├─ Crea mapas de bits (bloques e inodos)
    ├─ Crea inode raíz
    ├─ Crea usuario root con contraseña
    └─ Escribe todo en la partición
    │
    ▼
mkfsSu: login -user=root -pass=123 -id=p1
    │
    ▼
Login::execute() es llamado
    │
    ├─ Recupera tabla de usuarios de la partición
    ├─ Busca usuario por nombre
    ├─ Valida contraseña contra hash
    ├─ Crea sesión de usuario
    └─ Actualiza contexto global
    │
    ▼
Usuario autenticado exitosamente
```

### Creación Archivo

```
Usuario: mkfile -name=/archivo.txt -size=1024 -path=/root/disco.dsk
    │
    ▼
MkFile::execute() es llamado
    │
    ├─ Valida que usuario esté autenticado
    ├─ Parsea ruta del archivo
    ├─ Busca directorio padre
    │
    ├─ Encuentra inode libre
    ├─ Crea entrada de inode
    ├─ Asigna bloques de datos (según tamaño)
    │
    ├─ Actualiza mapa de bits de inodos
    ├─ Actualiza mapa de bits de bloques
    │
    ├─ Añade entrada en bloque de directorio padre
    └─ Escribe todos los cambios en disco
    │
    ▼
Archivo creado exitosamente
```

### Lectura de Archivo

```
Usuario: cat -path=/root/disco.dsk -file=/archivo.txt
    │
    ▼
Cat::execute() es llamado
    │
    ├─ Busca inode del archivo
    ├─ Valida permisos de lectura
    │
    ├─ Lee tabla de punteros de bloque en inode
    ├─ Por cada puntero de bloque:
    │   ├─ Loza bloque de datos del disco
    │   └─ Añade contenido a buffer
    │
    └─ Imprime contenido en consola
    │
    ▼
Contenido mostrado
```

---

## Compilación y Construcción

### Requisitos Previos
```bash
# Ubuntu/Debian
sudo apt-get install cmake g++ make nodejs npm

# macOS
brew install cmake gcc make node npm

# Fedora/RHEL
sudo dnf install cmake gcc-c++ make nodejs npm
```

### Pasos de Compilación

```bash
# 1. Navegar al directorio del proyecto
cd /home/mariano/MIA_1S2026_P1_202101149

# 2. Crear directorio de compilación
cd frontend/backend
mkdir build
cd build

# 3. Configurar compilación con CMake
cmake ..

# 4. Compilar código C++
make

# 5. Verificar ejecutable
ls -la extreamfs
```

### Flags de Compilación

```cmake
# En CMakeLists.txt:
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -O2")

# Debug:
set(CMAKE_CXX_FLAGS "-Wall -Wextra -g -O0")
```

### Instalación de Dependencias Frontend

```bash
cd frontend
npm install

# O instalar paquetes específicos:
npm install next react typescript
npm install -D tailwindcss postcss autoprefixer
npm install eslint @typescript-eslint/parser
```

---

## Detalles de Implementación

### 1. Gestor de Discos

**Responsabilidades:**
- Crear archivos de disco virtual
- Mantener tabla de discos disponibles
- Manejar lectura/escritura en sectores
- Implementar estrategias de ajuste (FF, BF, WF)

**Archivos clave:**
- `disk_creator.cpp`: Creación y manipulación de archivos
- `mbr.h`: Definición de estructura MBR
- `ebr.h`: Definición de estructura EBR

### 2. Sistema de Archivos EXT2

**Responsabilidades:**
- Leer y escribir SuperBloques
- Gestionar tabla de inodos
- Administrar bloques de datos
- Mantener mapas de bits

**Archivos clave:**
- `ext2_reader.cpp`: Lectura de estructuras EXT2
- `ext2_writer.cpp`: Escritura de estructuras EXT2
- `SuperBlock.cpp`: Manipulación de SuperBloque
- `EXT2Utils.h`: Funciones auxiliares

### 3. Gestor de Usuarios y Permisos

**Responsabilidades:**
- Autenticación de usuarios
- Gestión de grupos
- Aplicación de permisos POSIX
- Mantenimiento de sesiones

**Archivos clave:**
- `login.cpp`: Autenticación
- `logout.cpp`: Cierre de sesión
- `session_manager.cpp`: Gestión de sesiones
- `users_manager.h`: Tablas de usuarios/grupos
- `permissions.h`: Cálculo de permisos

### 4. Servidor HTTP

**Responsabilidades:**
- Procesar solicitudes HTTP (POST)
- Parsear JSON con comandos
- Ejecutar comandos en backend
- Retornar respuestas JSON

**Archivos clave:**
- `server.cpp`: Implementación de servidor
- `httplib.h`: Librería HTTP

**Endpoints:**
```
POST /command
{
    "command": "mkdisk -size=100 -unit=M -fit=FF -path=/root/disco.dsk"
}

Respuesta:
{
    "status": "success",
    "output": "Disco creado exitosamente"
}
```

### 5. Generadores de Reportes

**Tipos de reportes:**
- **MBR Report**: Visualiza tabla de particiones
- **SuperBloque**: Metadatos del filesystem
- **Mapa de Bits Bloques**: Estado de asignación
- **Mapa de Bits Inodos**: Inodos usados/libres
- **Árbol de Directorios**: Estructura jerárquica
- **Listado de Archivos**: Detalle de archivos

**Librería utilizada:**
- GraphViz para generar visualizaciones
- Exporta a PNG o JPG

### 6. Frontend Next.js

**Componentes principales:**
- `CommandPanel`: Entrada de comandos
- `Terminal`: Mostrar salida
- `FileExplorer`: Navegación de directorios
- `BlockViewer`: Visualización de bloques

**Hooks:**
- `useFilesystem`: Estado compartido del sistema

**Servicios:**
- `api.ts`: Cliente HTTP para comunicar con backend

---

## Variables de Sesión y Estado

```cpp
struct SessionContext {
    bool isAuthenticated;           // ¿Usuario autenticado?
    int currentUserID;              // ID del usuario actual
    int currentGroupID;             // ID del grupo actual
    std::string currentPartition;   // Partición montada
    std::string currentPath;        // Directorio actual
    int currentInode;               // Inode actual
};
```

---

## Algoritmos Clave

### Búsqueda de Inode Libre
```cpp
// Recorre mapa de bits de inodos
// Encuentra primer 0 (inodo libre)
// Marca como 1 (en uso)
// Retorna número de inode
```

### Búsqueda de Bloque Libre
```cpp
// First Fit: Primer bloque disponible
// Best Fit: Bloque que mejor se ajusta al tamaño
// Worst Fit: Bloque más grande disponible
```

### Lectura de Ruta
```cpp
// "/archivo.txt" → ["/", "archivo.txt"]
// "/dir1/dir2/archivo" → ["/", "dir1", "dir2", "archivo"]
// Busca cada componente en tabla de inodos
```

---

## Limitaciones Conocidas

1. **Tamaño máximo de archivo**: 4GB (límite de EXT2)
2. **Nombre de archivo**: Máximo 255 caracteres
3. **Profundidad de directorios**: Sin límite teórico
4. **Usuarios simultaneos**: Solo uno por partición montada
5. **Particiones lógicas**: Máximo 11 en disco extendido

---

## Performance

| Operación | Complejidad | Tiempo Estimado |
|-----------|-----------|-----------------|
| Crear disco | O(n) | < 1 segundo |
| Formatear partición | O(n) | < 2 segundos |
| Login/Logout | O(1) | < 100ms |
| Crear archivo | O(log n) | < 500ms |
| Leer archivo | O(n) | < 1 segundo |
| Buscar archivo | O(n) | < 1 segundo |
| Generar reporte | O(n) | < 2 segundos |

---

## Conclusiones Técnicas

EXTREAMFS es una implementación educativa completa de un sistema de archivos EXT2 que demuestra:

- ✓ Arquitectura en capas bien definida
- ✓ Separación clara entre frontend y backend
- ✓ Implementación de EXT2 desde cero
- ✓ Sistema completo de autenticación y permisos
- ✓ Interfaz moderna y responsiva
- ✓ Documentación técnica comprensiva

El proyecto sirve como base para aprender sobre:
- Sistemas de archivos
- Gestión de discos
- Autenticación y permisos
- Arquitectura cliente-servidor
- Desarrollo web moderno
