#ifndef MOVE_CMD_H
#define MOVE_CMD_H
#include <string>
class MoveCmd {
public:
    void execute(const std::string& path, const std::string& destino);
};
#endif