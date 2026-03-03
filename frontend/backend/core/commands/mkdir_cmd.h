#ifndef MKDIR_CMD_H
#define MKDIR_CMD_H
#include <string>
class MkdirCmd {
public:
    void execute(const std::string& path, bool createParents);
};
#endif