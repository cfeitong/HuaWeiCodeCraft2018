#ifndef CONSTANT_H_
#define CONSTANT_H_

#include "lib_io.h"

#include <string>

struct Info;
extern Info INFO;

int flavorid(const std::string &flavor);

extern const int CPU[20];

extern const int MEM[20];

#endif // CONSTANT_H_