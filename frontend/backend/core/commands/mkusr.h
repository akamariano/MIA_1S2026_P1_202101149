#ifndef MKUSR_H
#define MKUSR_H
#include <string>
class MkUsr {
public:
    void execute(const std::string& user,
                 const std::string& pass,
                 const std::string& grp);
};
#endif