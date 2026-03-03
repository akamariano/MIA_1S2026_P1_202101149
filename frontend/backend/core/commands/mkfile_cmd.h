#ifndef MKFILE_CMD_H
#define MKFILE_CMD_H
#include <string>
class MkfileCmd {
public:
    void execute(const std::string& path, bool createParents,
                 int size, const std::string& cont);
};
#endif