//
// Created by 陈斐童 on 13/04/2018.
//
#include "allocation.h"

#include <string>
#include <fstream>

using namespace std;

int main() {
    string base = "data/deploy_test_cases/case";
    for (int i = 1; i <= 6; i++) {
        string path = base + to_string(i) + ".txt";
    }

    return 0;
}

