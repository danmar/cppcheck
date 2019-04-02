
#include <fstream>
#include <iostream>

#include "type2.h"

int main(int argc, char **argv)
{
    const char *filename = argc==2 ? argv[1] : nullptr;

    if (!filename) {
        std::cout << "Invalid args, no filename\n";
        return 1;
    }

    std::ifstream f(filename);
    if (!f.is_open()) {
        std::cout << "failed to open file:" << filename << "\n";
        return 1;
    }

    std::string str((std::istreambuf_iterator<char>(f)),
                    std::istreambuf_iterator<char>());

    std::cout << generateCode2((const uint8_t *)str.data(), str.size()) << std::endl;

    return 0;
}

