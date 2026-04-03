#ifndef MKFS_H
#define MKFS_H
#include <string>
class Mkfs {
public:
    void execute(std::string id, std::string type = "full");
};
#endif