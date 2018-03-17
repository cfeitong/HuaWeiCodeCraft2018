#ifndef ALLOCATION_H_
#define ALLOCATION_H_

#include "constant.h"
#include <map>
#include <string>

using namespace std;

class Allocator {
  public:
    Allocator(int _cpu, int _mem) : cpu(_cpu), mem(_mem) {}

    int count(int phy_id, string flavor) {
        return this->result[phy_id-1][flavor];
    }

    int count_phy() {
        return this->result.size();
    }

    int count_vir(const string &flavor) {
        int ret = 0;
        for (auto &it : this->result) {
            ret += it.second[flavor];
        }
        return ret;
    }

    void alloc(string flavor) {
        int cpu_used = CPU[flavorid(flavor)];
        int mem_used = MEM[flavorid(flavor)];
        bool ok = false;
        for (auto &it : this->resource) {
            auto &c = it.second.first;
            auto &m = it.second.second;
            if (cpu_used <= c && mem_used <= m) {
                c -= cpu_used;
                m -= mem_used;
                this->result[it.first][flavor]++;
                ok = true;
                break;
            }
        }

        if (!ok) {
            this->resource[this->resource.size()] = {this->cpu, this->mem};
            alloc(flavor);
        }
    }

  private:
    map<int, pair<int, int>> resource;
    map<int, map<string, int>> result;
    int cpu, mem;
};

#endif // ALLOCATION_H_