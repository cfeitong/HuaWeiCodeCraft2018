#ifndef ALLOCATION_H_
#define ALLOCATION_H_

#include "constant.h"
#include <map>
#include <string>
#include <vector>

using namespace std;

class Allocator {
  public:
    Allocator(int _cpu, int _mem, const string &_type) : cpu(_cpu), mem(_mem), type(_type) {}

    int count(int phy_id, string flavor) {
        return this->result[phy_id-1][flavor];
    }

    map<string, int> count(int phy_id) {
        return this->result[phy_id];
    }

    int count_phy() const {
        return this->result.size();
    }

    int count_vir(const string &flavor) {
        int ret = 0;
        for (auto &it : this->result) {
            ret += it.second[flavor];
        }
        return ret;
    }

    // used for offline algorithm
    // after adding all elements, call this->compute() is a must
    void add_elem(const string &flavor) {
        this->elems.push_back(flavor);
    }

    void reset() {
        *this = Allocator(cpu, mem, type);
    }

    // best fit decreasing
    void compute() {
        sort(this->elems.begin(), this->elems.end(),
             [&](const string &fa, const string &fb) {
            int aid = flavorid(fa);
            int bid = flavorid(fb);
            pair<int, int> a, b;
            if (this->type == "CPU") {
                a.first = CPU[aid];
                a.second = MEM[aid];
                b.first = CPU[bid];
                b.second = MEM[bid];
            } else if (this->type == "MEM") {
                a.first = MEM[aid];
                a.second = CPU[aid];
                b.first = MEM[bid];
                b.second = CPU[bid];
            }
            if (a.first == b.first) {
                return a.second > b.second;
            } else {
                return a.first > b.second;
            }
        });
        vector<pair<int, int>> bins(this->elems.size(), {0, 0});
        int lim1 = 0, lim2 = 0;
        if (this->type == "CPU") {
            lim1 = this->cpu;
            lim2 = this->mem;
        } else if (this->type == "MEM") {
            lim1 = this->mem;
            lim2 = this->cpu;
        }
        int res = 0;
        for (const string &elem : elems) {
            int id = flavorid(elem);
            int c1 = 0, c2 = 0;
            if (this->type == "CPU") {
                c1 = CPU[id];
                c2 = MEM[id];
            } else if (this->type == "MEM") {
                c1 = MEM[id];
                c2 = CPU[id];
            }
            int m1 = 0x3f3f3f3f;
            int m2 = 0x3f3f3f3f;
            int idx = 0, i = 0;
            for (i = 0; i < res; i++) {
                if (bins[i].first + c1 <= lim1 && bins[i].second + c2 <= lim2) {
                    if (lim1 - bins[i].first - c1 == m1) {
                        if (lim2 - bins[i].second - c2 < m2) {
                            idx = i;
                            m2 = lim2 - bins[i].second - c2;
                            break;
                        }
                    } else if (lim1 - bins[i].first - c1 < m1) {
                        idx = i;
                        m1 = lim1 - bins[i].first - c1;
                        break;
                    }
                }
            }
            if (i == res) idx = ++res;
            this->result[idx][elem]++;
            bins[idx].first += c1;
            bins[idx].second += c2;
        }
    }

    void alloc(const string &flavor) {
        int id = flavorid(flavor);
        int cpu_used = CPU[id];
        int mem_used = MEM[id];
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

    map<string, int> flavor_count() {
        map<string, int> ret;
        for (const auto &phy : this->result) {
            for (const auto &vir : phy.second) {
                ret[vir.first] += vir.second;
            }
        }
        return ret;
    };

  private:
    vector<string> elems;
    map<int, pair<int, int>> resource;
    map<int, map<string, int>> result;
    int cpu, mem;
    string type;
};

#endif // ALLOCATION_H_