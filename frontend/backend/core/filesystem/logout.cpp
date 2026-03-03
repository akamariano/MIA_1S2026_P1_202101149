#include "logout.h"
#include "../core/filesystem/session_manager.h"
#include <iostream>

using namespace std;

void Logout::execute() {

    if (!SessionManager::isActive()) {
        cout << "ERROR: No hay sesión activa\n";
        return;
    }

    string user = SessionManager::get().username;
    SessionManager::logout();
    cout << "OK: Sesión de '" << user << "' cerrada correctamente\n";
}