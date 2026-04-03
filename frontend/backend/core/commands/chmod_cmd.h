#ifndef CHMOD_CMD_H
#define CHMOD_CMD_H
#include <string>
class ChmodCmd {
public:
    void execute(const std::string& path,
                 const std::string& ugo,
                 bool recursive);
};
#endif