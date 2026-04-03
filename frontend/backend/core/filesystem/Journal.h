#ifndef JOURNAL_H
#define JOURNAL_H

#pragma pack(push, 1)

struct Information {
    char  i_operation[10];
    char  i_path[32];
    char  i_content[64];
    float i_date;
};

struct Journal {
    int         j_count;
    Information j_content;
};

#pragma pack(pop)

#endif