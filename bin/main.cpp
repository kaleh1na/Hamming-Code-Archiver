#include "lib/HamArc.cpp"

#include <iostream>


int main(int argc, char** argv) {
    Options opt;
    Parse(argc, argv, opt);
    if (!Parse(argc, argv, opt)) {
        return -1;
    }
    if (!ArchivatorManager(opt)) {
        return -1;
    }
    return 0;
}

