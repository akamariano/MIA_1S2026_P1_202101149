# Diagrama de Flujo - EXTREAMFS

| Nombre | Carnet|
|----------|-------|
| Mariano Roberto Rac Noguera| 202101149 |

---

## Tabla de Contenidos
1. [Descripción General](#descripción-general)
2. [Flujo Principal del Sistema](#flujo-principal-del-sistema)
3. [Flujo de Inicialización](#flujo-de-inicialización)
4. [Flujo de Operaciones de Disco](#flujo-de-operaciones-de-disco)
5. [Flujo de Operaciones de Sistema de Archivos](#flujo-de-operaciones-de-sistema-de-archivos)
6. [Flujo de Autenticación y Control de Acceso](#flujo-de-autenticación-y-control-de-acceso)
7. [Flujo de Generación de Reportes](#flujo-de-generación-de-reportes)

---

## Descripción General

El diagrama de flujo de EXTREAMFS representa el funcionamiento integral del sistema, mostrando cómo los datos se desplazan a través de las distintas capas y módulos. El sistema opera bajo un modelo cliente-servidor, donde la interfaz web (frontend) comunica con el servidor backend mediante API HTTP para ejecutar operaciones sobre el sistema de archivos virtual.

![Flujo de Datos](https://images.unsplash.com/photo-1551288049-bebda4e38f71?w=600&h=400&fit=crop)
*Arquitectura de flujo de datos del sistema*

---

## Flujo Principal del Sistema

```
┌─────────────────────────────────────────────────────────────────┐
│                    USUARIO / INTERFAZ WEB                       │
│                     (Frontend - Next.js)                        │
└──────────────────────────┬──────────────────────────────────────┘
                           │
                           ▼
        ┌──────────────────────────────────────────┐
        │   API REST (Servidor Backend C++)        │
        │   ├─ Puerto 8888 (HTTP)                  │
        │   └─ Procesamiento de Comandos           │
        └──────────────────────────────────────────┘
                           │
                ┌──────────┼──────────┐
                │          │          │
                ▼          ▼          ▼
         ┌────────────┬────────────┬──────────────┐
         │  Gestor    │  Sistema   │  Reportes    │
         │   Discos   │ Archivos   │     PDF      │
         │   (MBR/    │  (EXT2)    │              │
         │    EBR)    │            │              │
         └────────────┴────────────┴──────────────┘
                │          │              │
                ▼          ▼              ▼
         ┌──────────────────────────────────────┐
         │   ALMACENAMIENTO - Archivos .dsk     │
         │   (Sistema de Archivos Virtual)      │
         └──────────────────────────────────────┘
```

![Arquitectura del Sistema](https://images.unsplash.com/photo-1517694712202-14dd9538aa97?w=600&h=400&fit=crop)
*Componentes arquitectónicos del sistema*

---

## Flujo de Inicialización

```
INICIO
  │
  ├─ Cargar Frontend (Next.js)
  │   ├─ Compilar componentes React/TypeScript
  │   ├─ Conectar con API Backend
  │   └─ Mostrar Terminal Interactiva
  │
  └─ Inicializar Servidor Backend
      ├─ Compilar código C++ (CMake)
      ├─ Crear carpeta de discos virtuales
   

![Inicialización del Sistema](https://images.unsplash.com/photo-1553531889-e6cf89d18e40?w=600&h=400&fit=crop)
*Proceso de startup del servidor*   ├─ Cargar discos existentes (.dsk)
      ├─ Inicializar gestor de sesiones
      └─ Escuchar en puerto 8888
```

---

## Flujo de Operaciones de Disco

```
USUARIO INGRESA COMANDO (mkdisk, rmdisk, fdisk, mount)
  │
  ├─ Validación de Sintaxis
  │   ├─ ¿Parámetros válidos?
  │   └─ NO → Error
  │
  ├─ Procesamiento en Backend
  │   │
  │   ├─ MKDISK: Crear nuevo disco virtual
  │   │   ├─ Generar estructura MBR
  │   │   ├─ Asignar espacio en disco (.dsk)
  │   │   └─ Guardar metadatos
  │   │
  │   ├─ FDISK: Particionar disco
  │   │   ├─ Leer MBR existente
  │   │   ├─ Crear partición primaria/lógica
  │   │   ├─ Escribir EBR si aplica
  │   │   └─ Actualizar estructura
  │   │
  │   ├─ MOUNT: Montar partición
  │   │   ├─ Verificar disponibilidad
  │   │   ├─ Cargar estructura EXT2
  │   │   ├─ Registrar en gestor de montaje
  │   │   └─ Activar para operaciones
  │   │
  │   └─ RMDISK: Eliminar disco
  │       ├─ Desmontar particiones activas
  │       ├─ Eliminar archivo .dsk
  │       └─ Actualizar registro
  │
  └─ Respuesta al Cliente
      ├─ Si éxito → Confirmar operación
      └─ Si error → Mensaje de error descriptivo
```

---

## Flujo de Operaciones de Sistema de Archivos

```
USUARIO INGRESA COMANDO (mkfs, mkdir, mkfile, cat, rm, cp, etc.)
  │
  ├─ Validación de Sesión
  │   ├─ ¿Usuario autenticado?
  │   ├─ ¿Sistema montado?
  │   └─ NO → Rechazar
  │
  ├─ Validación de Permisos
  │   ├─ Verificar permisos del directorio padre
  │   ├─ Verificar permisos de usuario/grupo
  │   └─ Rechazar si no autorizado
  │
  ├─ Procesamiento de Operación
  │   │
  │   ├─ MKFS: Formatear partición
  │   │   ├─ Crear Super Bloque
  │   │   ├─ Inicializar tabla de inodos
  │   │   ├─ Crear bitmap de bloques e inodos
  │   │   └─ Crear directorio raíz (/)
  │   │
  │   ├─ MKDIR: Crear directorio
  │   │   ├─ Asignar nuevo inodo
  │   │   ├─ Crear entrada en directorio padre
  │   │   ├─ Establecer permisos
  │   │   └─ Actualizar bitmap
  │   │
  │   ├─ MKFILE: Crear archivo
  │   │   ├─ Asignar inodo
  │   │   ├─ Crear entrada en directorio
  │   │   ├─ Disponibilizar bloques
  │   │   └─ Escribir contenido si proporcionado
  │   │
  │   ├─ CAT: Leer archivo
  │   │   ├─ Localizar inodo
  │   │   ├─ Leer bloques de datos
  │   │   └─ Mostrar contenido
  │   │
  │   ├─ RM: Eliminar archivo/directorio
  │   │   ├─ Verificar si no está vacío
  │   │   ├─ Liberar inodo
  │   │   ├─ Liberar bloques de datos
  │   │   ├─ Actualizar bitmaps
  │   │   └─ Remover entrada del directorio
  │   │
  │   └─ CP: Copiar archivo
  │       ├─ Leer archivo origen
  │       ├─ Crear nuevo inodo
  │       ├─ Asignar bloques necesarios
  │       ├─ Copiar datos
  │       └─ Crear entrada en directorio destino
  │
  └─ Sincronizar Cambios
      ├─ Actualizar en memoria
      ├─ Escribir en archivo .dsk
      └─ Confirmar al cliente
```

---

## Flujo de Autenticación y Control de Acceso

```
USUARIO INICIA SESIÓN (login)
  │
  ├─ Validar Credenciales
  │   ├─ Usuario existe en /etc/passwd?
  │   ├─ Contraseña coincide?
  │   └─ NO → Rechazar acceso
  │
  ├─ Cargar Información de Usuario
  │   ├─ UID, GID
  │   ├─ Grupos secundarios
  │   ├─ Directorio home
  │   └─ Shell por defecto
  │
  ├─ Crear Sesión
  │   ├─ Generar token de sesión
  │   ├─ Almacenar en gestor de sesiones
  │   ├─ Registrar tiempo de login
  │   └─ Establecer contexto de usuario
  │
  └─ Usuario Autenticado
      └─ Puede ejecutar comandos dentro de permisos
```

---

## Flujo de Generación de Reportes

```
USUARIO SOLICITA REPORTE (rep)
  │
  ├─ Validar Tipo de Reporte
  │   ├─ mbr, disk, inode, block, bm, sb, file, ls
  │   └─ NO → Error
  │
  ├─ Recopilar Datos
  │   │
  │   ├─ Si MBR: Leer estructura Master Boot Record
  │   ├─ Si DISK: Leer información completa del disco
  │   ├─ Si INODO: Leer tabla de inodos
  │   ├─ Si BLOCK: Leer información de bloques
  │   ├─ Si BM: Leer bitmaps de inodos y bloques
  │   ├─ Si SB: Leer estructura del Super Bloque
  │   ├─ Si FILE: Leer contenido de archivo
  │   └─ Si LS: Listar contenido de directorio
  │
  ├─ Generar Salida
  │   ├─ Crear archivo temporal con datos
  │   ├─ Procesar con GraphViz
  │   ├─ Generar imagen gráfica (PNG/JPG)
  │   └─ Almacenar en carpeta de reportes
  │
  ├─ Presentar Resultado
  │   ├─ Mostrar visualización en terminal
  │   ├─ Proporcionar ruta del archivo
  │

![Generación de Reportes](https://images.unsplash.com/photo-1355997764557-a3a3d7458e31?w=600&h=400&fit=crop)
*Visualización de reportes y análisis*   └─ Permitir descarga desde interfaz
  │
  └─ FIN
```

---

## Notas Adicionales

- El sistema utiliza una **arquitectura no bloqueante** para optimizar el manejo simultáneo de múltiples solicitudes
- Las operaciones de disco se sincronizan con el almacenamiento (.dsk) para garantizar consistencia
- El control de acceso sigue el modelo de **permisos POSIX** (lectura, escritura, ejecución para usuario, grupo y otros)
- Los reportes se generan bajo demanda para reflejar el estado actual del sistema
