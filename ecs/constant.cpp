#include "constant.h"
#include <cstdio>
#include <string>

using namespace std;

int flavorid(const string &flavor) {
    int id;
    sscanf(flavor.c_str(), "flavor%d", &id);
    return id;
}

Info INFO(nullptr);

const int CPU[20] = {
        -1, 1, 1, 1, 2, 2, 2, 4, 4, 4, 8, 8, 8, 16, 16, 16,
};

const int MEM[20] = {
        -1, 1, 2, 4, 2, 4, 8, 4,
        8, 16, 8, 16, 32, 16, 32, 64,
};
