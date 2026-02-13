#include <iostream>
#include "commands/mkdisk.h"
#include <cstdlib>
#include <ctime>

int main(int argc, char* argv[]) {

    srand(time(nullptr));

    if (argc < 6) {
        std::cout << "Uso: ./extreamfs mkdisk <size> <unit> <fit> <path>\n";
        return 1;
    }

    std::string command = argv[1];

    if (command == "mkdisk") {

        int size = std::stoi(argv[2]);
        char unit = argv[3][0];
        char fit = argv[4][0];
        std::string path = argv[5];

        MkDisk mk;
        mk.execute(size, unit, fit, path);
    }

    return 0;
}
