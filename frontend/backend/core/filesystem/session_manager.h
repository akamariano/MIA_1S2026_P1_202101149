#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include <string>

struct Session {
    bool   active      = false;
    std::string username;
    std::string groupname;
    int    uid         = -1;
    int    gid         = -1;
    std::string partitionId;  // ej: "491A"
};

class SessionManager {
private:
    static Session currentSession;

public:
    static bool isActive() { 
        return currentSession.active; 
    }

    static void login(const std::string& user, const std::string& group,
                      int uid, int gid, const std::string& id) {
        currentSession.active      = true;
        currentSession.username    = user;
        currentSession.groupname   = group;
        currentSession.uid         = uid;
        currentSession.gid         = gid;
        currentSession.partitionId = id;
    }

    static void logout() {
        currentSession = Session();
    }

    static const Session& get() { 
        return currentSession; 
    }

    static bool isRoot() { 
        return currentSession.active && currentSession.username == "root"; 
    }
};

// Definición del static en session_manager.cpp
// Session SessionManager::currentSession;

#endif