#ifndef PERMISSIONS_H
#define PERMISSIONS_H

#include "../core/filesystem/Inode.h"
#include "../core/filesystem/session_manager.h"

// Categorías UGO
enum class PermCategory { USER, GROUP, OTHER };

// Bits de permiso octal
enum class PermType { READ = 4, WRITE = 2, EXECUTE = 1 };

class Permissions {
public:

    // Determina la categoría del usuario actual respecto al inodo
    static PermCategory getCategory(const Inode& inode) {
        const Session& s = SessionManager::get();
        if (s.uid == inode.i_uid)  return PermCategory::USER;
        if (s.gid == inode.i_gid)  return PermCategory::GROUP;
        return PermCategory::OTHER;
    }

    // Verifica si el usuario actual tiene el permiso solicitado
    static bool hasPermission(const Inode& inode, PermType perm) {

        // Root siempre tiene permisos 777
        if (SessionManager::isRoot()) return true;

        int p = inode.i_perm; // ej: 664

        // Extraer dígitos UGO
        int u = (p / 100) % 10;
        int g = (p / 10)  % 10;
        int o =  p        % 10;

        int permBit = (int)perm;
        PermCategory cat = getCategory(inode);

        switch (cat) {
            case PermCategory::USER:  return (u & permBit) != 0;
            case PermCategory::GROUP: return (g & permBit) != 0;
            case PermCategory::OTHER: return (o & permBit) != 0;
        }
        return false;
    }

    static bool canRead(const Inode& inode) {
        return hasPermission(inode, PermType::READ);
    }

    static bool canWrite(const Inode& inode) {
        return hasPermission(inode, PermType::WRITE);
    }

    static bool canExecute(const Inode& inode) {
        return hasPermission(inode, PermType::EXECUTE);
    }
};

#endif