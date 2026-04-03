#ifndef JOURNALING_CMD_H
#define JOURNALING_CMD_H
#include <string>
class JournalingCmd {
public:
    void execute(const std::string& id);
};
#endif