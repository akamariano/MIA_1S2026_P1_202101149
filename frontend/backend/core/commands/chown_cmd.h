#ifndef CHOWN_CMD_H
#define CHOWN_CMD_H
#include <string>
class ChownCmd {
public:
    void execute(const std::string& path,
                 const std::string& usuario,
                 bool recursive);
};
#endif