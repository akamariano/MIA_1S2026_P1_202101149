#ifndef COPY_CMD_H
#define COPY_CMD_H
#include <string>
class CopyCmd {
public:
    void execute(const std::string& path, const std::string& destino);
};
#endif