#ifndef CONSTANT_H_
#define CONSTANT_H_

#include <string>
#include <vector>
using namespace std;

int flavorid(const std::string &flavor);
vector<double> get_pretrained(string flavor);

const int CPU[20] = {
        -1, 1, 1, 1, 2, 2, 2, 4, 4, 4, 8, 8, 8, 16, 16, 16,
};

const int MEM[20] = {
        -1, 1, 2, 4, 2, 4, 8, 4,
        8, 16, 8, 16, 32, 16, 32, 64,
};

#endif // CONSTANT_H_