#ifndef RENAME_CMD_H
#define RENAME_CMD_H
#include <string>
class RenameCmd {
public:
    void execute(const std::string& path, const std::string& newName);
};
#endif