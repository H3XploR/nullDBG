// main.cpp
#include "Debugger.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <program to debug>\n";
        return -1;
    }

    Debugger dbg(argv[1]);
    dbg.run();
    return 0;
}

