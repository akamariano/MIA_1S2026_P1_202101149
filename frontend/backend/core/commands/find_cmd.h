#ifndef FIND_CMD_H
#define FIND_CMD_H
#include <string>
class FindCmd {
public:
    void execute(const std::string& path, const std::string& name);
};
#endif