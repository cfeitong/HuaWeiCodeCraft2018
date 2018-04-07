#include "constant.h"
#include <cstdio>
#include <string>

using namespace std;

int flavorid(string flavor) {
    int id;
    sscanf(flavor.c_str(), "flavor%d", &id);
    return id;
}