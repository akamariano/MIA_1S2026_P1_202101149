# Manual de Usuario - EXTREAMFS

| Nombre | Carnet|
|----------|-------|
| Mariano Roberto Rac Noguera| 202101149 |

---

## Tabla de Contenidos
1. [Introducción](#introducción)
2. [Requisitos Previos](#requisitos-previos)
3. [Instalación](#instalación)
4. [Iniciar la Aplicación](#iniciar-la-aplicación)
5. [Interfaz de Usuario](#interfaz-de-usuario)
6. [Guía de Uso Paso a Paso](#guía-de-uso-paso-a-paso)
7. [Operaciones Comunes](#operaciones-comunes)
8. [Solución de Problemas](#solución-de-problemas)

---

## Introducción

**EXTREAMFS** es un simulador de sistema de archivos basado en la estructura EXT2. La aplicación proporciona una interfaz gráfica moderna construida con Next.js y un servidor backend en C++ que maneja todas las operaciones del sistema de archivos.

Con EXTREAMFS puedes:
- Crear y gestionar discos virtuales
- Particionar discos con MBR y EBR
- Implementar sistemas de archivos EXT2
- Gestionar usuarios y grupos
- Crear, leer, modificar y eliminar archivos
- Generar reportes del estado del sistema
- Visualizar la estructura del sistema de archivos

---

## Requisitos Previos

### Hardware
- Procesador: Intel o compatible (x86_64)
- RAM: Mínimo 2GB
- Espacio en disco: 500MB disponibles
- Pantalla: Resolución mínima 1024x768

### Software
- **Node.js** v16 o superior
- **npm** v7 o superior
- **CMake** v3.22 o superior
- **Compilador C++** (g++ o clang)
- **Linux/Mac/WSL** (Windows Subsystem for Linux)

---

## Instalación

### Paso 1: Descargar el Proyecto
```bash
cd /home/mariano/MIA_1S2026_P1_202101149
```

### Paso 2: Instalar Dependencias del Backend
```bash
cd frontend/backend
mkdir build
cd build
cmake ..
make
```

### Paso 3: Instalar Dependencias del Frontend
```bash
cd ../..
npm install
```

---

## Iniciar la Aplicación

### Opción 1: Ejecución Completa (Recomendado)

**Terminal 1 - Iniciar Backend:**
```bash
cd frontend/backend/build
./extreamfs --server 8080
```

**Terminal 2 - Iniciar Frontend:**
```bash
cd frontend
npm run dev
```

Luego abre tu navegador en: **http://localhost:3000**

### Opción 2: Modo Consola
```bash
cd frontend/backend/build
./extreamfs
```

---

## Interfaz de Usuario

La interfaz de EXTREAMFS está dividida en varios componentes principales:

![Interfaz Principal](./Documentacion/images/INTERFAZ.png)
*Figura 1: Interfaz principal de la aplicación*

### Componentes Principales

#### 1. **Panel de Comandos** (Arriba a la izquierda)
- Entrada de texto para escribir comandos
- Botón de envío
- Historial de comandos recientes

#### 2. **Terminal de Salida** (Centro)
- Muestra los resultados de los comandos ejecutados
- Historial de operaciones
- Mensajes de suceso o error

#### 3. **Explorador de Archivos** (Derecha)
- Visualización de la estructura de directorios
- Navegación por carpetas
- Vista de propiedades de archivos

#### 4. **Visualizador de Bloques** (Abajo)
- Visualización de bloques del disco
- Estado de asignación de bloques
- Información de inodos

---

## Guía de Uso Paso a Paso

### Paso 1: Crear un Disco Virtual
```bash
mkdisk -size=100 -unit=M -fit=FF -path=/root/disco1.dsk
```

**Parámetros:**
- `-size`: Tamaño del disco (número)
- `-unit`: Unidad (K=Kilobytes, M=Megabytes, G=Gigabytes)
- `-fit`: Estrategia (FF=First Fit, BF=Best Fit, WF=Worst Fit)
- `-path`: Ruta donde se guardará el archivo

### Paso 2: Particionar el Disco
```bash
fdisk -size=50 -unit=M -type=P -fit=FF -delete=0 -name=p1 -path=/root/disco1.dsk
```

**Parámetros:**
- `-size`: Tamaño de la partición
- `-type`: Tipo (P=Primaria, E=Extendida, L=Lógica)
- `-fit`: Estrategia de ajuste
- `-name`: Nombre de la partición
- `-path`: Ruta del disco

### Paso 3: Crear Sistema de Archivos
```bash
mkfs -type=ext2 -fs=ext2 -id=p1 -path=/root/disco1.dsk
```

**Parámetros:**
- `-type`: Tipo de formato (ext2)
- `-fs`: Sistema de archivos
- `-id`: ID de la partición
- `-path`: Ruta del disco

### Paso 4: Realizar Login
```bash
login -user=root -pass=123 -id=p1
```

**Parámetros:**
- `-user`: Nombre de usuario
- `-pass`: Contraseña
- `-id`: ID de la partición

### Paso 5: Crear Directorio
```bash
mkdir -name=/carpeta -path=/root/disco1.dsk
```

### Paso 6: Crear Archivo
```bash
mkfile -name=/archivo.txt -size=1024 -path=/root/disco1.dsk
```

### Paso 7: Ver Contenido de Archivo
```bash
cat -path=/root/disco1.dsk -file=/archivo.txt
```

---

## Operaciones Comunes

### Gestión de Usuarios

**Crear Usuario:**
```bash
mkusr -user=juan -pass=pass123 -id=p1
```

**Crear Grupo:**
```bash
mkgrp -name=developers -id=p1
```

**Cambiar Grupo de Usuario:**
```bash
chgrp -user=juan -group=developers -id=p1
```

**Eliminar Usuario:**
```bash
rmusr -user=juan -id=p1
```

**Eliminar Grupo:**
```bash
rmgrp -name=developers -id=p1
```

### Gestión de Discos

**Eliminar Disco:**
```bash
rmdisk -path=/root/disco1.dsk
```

**Montar Partición:**
```bash
mount -path=/root/disco1.dsk -name=p1
```

### Reportes

**Generar Reporte MBR:**
```bash
rep -name=mbr -path=/root/disco1.dsk -id=p1
```

**Reporte de Bloques:**
```bash
rep -name=bm -path=/root/disco1.dsk -id=p1
```

**Reporte de Inodos:**
```bash
rep -name=bit -path=/root/disco1.dsk -id=p1
```

**Reporte de Árbol de Directorios:**
```bash
rep -name=tree -path=/root/disco1.dsk -id=p1
```

**Reporte de Listado de Archivos:**
```bash
rep -name=ls -path=/root/disco1.dsk -id=p1
```

---

## Pantallazos de Ejemplo

### Consola en Ejecución
![Consola](./Documentacion/images/CONSOLAS.png)
*Figura 2: Consola ejecutando comandos*

### Reportes del Sistema

#### Reporte MBR (Master Boot Record)
![Reporte MBR](./Documentacion/images/disco1_mbr.jpg)
*Figura 3: Estructura del MBR del disco*

#### Reporte SuperBloque
![SuperBloque](./Documentacion/images/disco1_sb.jpg)
*Figura 4: Información del SuperBloque EXT2*

#### Mapa de Bits de Bloques
![Bloques](./Documentacion/images/disco1_block.jpg)
*Figura 5: Mapa de bits de bloques asignados*

#### Mapa de Bits de Inodos
![Inodos](./Documentacion/images/disco1_inode.jpg)
*Figura 6: Mapa de bits de inodos*

#### Árbol de Directorios
![Árbol](./Documentacion/images/disco1_tree.jpg)
*Figura 7: Estructura jerárquica de directorios*

#### Listado de Archivos y Documentos
![Listado](./Documentacion/images/disco1_ls_docs.jpg)
*Figura 8: Listado detallado de archivos y carpetas*

---

## Solución de Problemas

### El servidor no inicia
**Problema:** Error al conectar con el puerto 8080
```
Solución:
1. Verifica que el puerto 8080 esté disponible
2. Usa otro puerto: ./extreamfs --server 8081
3. Termina cualquier proceso en el puerto 8080
```

### Interfaz no carga
**Problema:** La página localhost:3000 no abre
```
Solución:
1. Verifica que npm run dev está ejecutándose
2. Recarga la página (Ctrl+R o Cmd+R)
3. Revisa la consola del navegador (F12)
4. Comprueba que no hay bloqueadores de puertos
```

### Comando rechazado
**Problema:** Error "Usuario no autenticado"
```
Solución:
1. Realiza login primero con: login -user=root -pass=123 -id=p1
2. Verifica que la partición exista y esté formateada
3. Comprueba la sintaxis del comando
```

### Disco no se monta
**Problema:** Error al montar partición
```
Solución:
1. Crea primero el sistema de archivos con mkfs
2. Verifica que la ruta del disco sea correcta
3. Asegúrate de tener permisos de lectura/escritura
```

### Espacio insuficiente
**Problema:** No se pueden crear más archivos
```
Solución:
1. Crea un disco más grande
2. Elimina archivos no necesarios
3. Revisa el espacio disponible con reportes
```

---

## Atajos de Teclado

| Atajo | Función |
|-------|---------|
| Enter | Ejecutar comando |
| Ctrl+C | Cancelar operación |
| Ctrl+L | Limpiar terminal |
| ↑/↓ | Navegar historial de comandos |

---

## Consejos Útiles

1. **Crea un disco de prueba pequeño** (10-50 MB) para empezar
2. **Usa nombres descriptivos** para discos y particiones
3. **Guarda los reportes** para diagnosticar problemas
4. **Realiza login con root** antes de operar en el sistema
5. **Verifica el árbol de directorios** antes de buscar archivos
6. **Respalda tus discos** copiando los archivos .dsk

