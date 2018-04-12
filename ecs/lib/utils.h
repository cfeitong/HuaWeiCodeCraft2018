//
// Created by 陈斐童 on 12/04/2018.
//

#ifndef ECS_UTILS_H
#define ECS_UTILS_H

#include <iostream>
#include <vector>

using namespace std;

template<typename T>
void print_vector(const vector<T> &v) {
    for (const auto &t : v) {
        cout << t << " ";
    }
    cout << endl;
}
#endif //ECS_UTILS_H
