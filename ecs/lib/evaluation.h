//
// Created by 陈斐童 on 21/03/2018.
//

#ifndef ECS_EVALUATION_H
#define ECS_EVALUATION_H

#include "allocation.h"
#include "lib_io.h"
#include <string>

std::pair<double, double> evaluate(
        Allocator &alloc,
        Info &meta,
        string test_file
);

#endif //ECS_EVALUATION_H
