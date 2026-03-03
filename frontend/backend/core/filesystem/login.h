#ifndef LOGIN_H
#define LOGIN_H
#include <string>

class Login {
public:
    void execute(const std::string& user, 
                 const std::string& pass, 
                 const std::string& id);
};
#endif