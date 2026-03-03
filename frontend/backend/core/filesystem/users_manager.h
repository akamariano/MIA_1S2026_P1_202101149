#ifndef USERS_MANAGER_H
#define USERS_MANAGER_H

#include <string>
#include <vector>
#include <sstream>

struct GroupEntry {
    int    gid;
    std::string name;
    bool   active; // false si id=0
};

struct UserEntry {
    int    uid;
    std::string group;
    std::string username;
    std::string password;
    bool   active; // false si id=0
};

struct UsersData {
    std::vector<GroupEntry> groups;
    std::vector<UserEntry>  users;
};

// Parsea el contenido de users.txt
inline UsersData parseUsers(const std::string& content) {
    UsersData data;
    std::istringstream stream(content);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.empty()) continue;

        std::vector<std::string> fields;
        std::stringstream ss(line);
        std::string field;
        while (std::getline(ss, field, ',')) {
            while (!field.empty() && field.front() == ' ') field.erase(0,1);
            while (!field.empty() && field.back()  == ' ') field.pop_back();
            fields.push_back(field);
        }

        if (fields.size() < 3) continue;

        int  id   = std::stoi(fields[0]);
        char type = fields[1][0];

        if (type == 'G') {
            GroupEntry g;
            g.gid    = id;
            g.name   = fields[2];
            g.active = (id > 0);
            data.groups.push_back(g);
        }
        else if (type == 'U' && fields.size() >= 5) {
            UserEntry u;
            u.uid      = id;
            u.group    = fields[2];
            u.username = fields[3];
            u.password = fields[4];
            u.active   = (id > 0);
            data.users.push_back(u);
        }
    }
    return data;
}

// Serializa UsersData a string para escribir en disco
inline std::string serializeUsers(const UsersData& data) {
    std::string result;
    // Primero grupos, luego usuarios — en orden de inserción
    // Reconstruir mezclados como estaban originalmente es complejo,
    // así que guardamos grupos primero luego usuarios
    for (auto& g : data.groups) {
        result += std::to_string(g.gid) + ",G," + g.name + "\n";
    }
    for (auto& u : data.users) {
        result += std::to_string(u.uid) + ",U," +
                  u.group + "," + u.username + "," + u.password + "\n";
    }
    return result;
}

// Obtiene el siguiente ID disponible para grupos
inline int nextGroupId(const UsersData& data) {
    int maxId = 0;
    for (auto& g : data.groups)
        if (g.gid > maxId) maxId = g.gid;
    return maxId + 1;
}

// Obtiene el siguiente ID disponible para usuarios
inline int nextUserId(const UsersData& data) {
    int maxId = 0;
    for (auto& u : data.users)
        if (u.uid > maxId) maxId = u.uid;
    return maxId + 1;
}

#endif