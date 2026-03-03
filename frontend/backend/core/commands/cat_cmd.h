#ifndef CAT_CMD_H
#define CAT_CMD_H
#include <string>
#include <vector>
class CatCmd {
public:
    void execute(const std::vector<std::string>& files);
};
#endif