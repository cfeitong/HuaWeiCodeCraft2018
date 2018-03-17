#ifndef CONSTANT_H_
#define CONSTANT_H_

#include <string>

int flavorid(std::string flavor);

const int CPU[20] = {
    -1, 1, 1, 1, 2, 2, 2, 4, 4, 4, 8, 8, 8, 16, 16, 16,
};

const int MEM[20] = {
    -1,    1024,  2048, 4096,  2048,  4096,  8192,  4096,
    8192, 16384, 8192, 16384, 32768, 16384, 32768, 65536,
};

#endif // CONSTANT_H_