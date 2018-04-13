//
// Created by 陈斐童 on 13/04/2018.
//
#include "allocation.h"

#include <string>
#include <cstdio>

using namespace std;

int main() {
    string base = "data/deploy_test_cases/case";
    for (int i = 1; i <= 6; i++) {
        string path = base + to_string(i) + ".txt";
        FILE *f = fopen(path.c_str(), "r");
        int cpu, mem, disk;
        fscanf(f, "%d%d%d", &cpu, &mem, &disk);
        char tmp[10]; fscanf(f, "%s", tmp); string opt(tmp);
        int cnt; fscanf(f, "%d", &cnt);
        map<string, int> res;
        Allocator alloc(cpu, mem, opt);
        for (int j = 0; j < cnt; j++) {
            int num; fscanf(f, "%s:%d", tmp, &num);
            string flavor(tmp);
            res[flavor] = num;
            for (int k = 0; k < num; k++) alloc.add_elem(flavor);
        }
        alloc.compute();
        printf("phy machine count: %d\n", alloc.count_phy());
    }

    return 0;
}

