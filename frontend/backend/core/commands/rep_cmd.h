#ifndef REP_CMD_H
#define REP_CMD_H
#include <string>

class RepCmd {
public:
    void execute(const std::string& name,
                 const std::string& path,
                 const std::string& id,
                 const std::string& pathFileLs);
};
#endif