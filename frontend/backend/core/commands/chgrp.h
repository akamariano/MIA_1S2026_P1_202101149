#ifndef CHGRP_H
#define CHGRP_H
#include <string>
class ChGrp {
public:
    void execute(const std::string& user, const std::string& grp);
};
#endif