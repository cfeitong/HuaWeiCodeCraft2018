//
// Created by 陈斐童 on 27/03/2018.
//

#include "allocation.h"
#include <iostream>
#include <algorithm>
#include <cmath>

int Allocator::count(int phy_id, const string &flavor) {
    int id = phy_id - 1;
    if (this->result.find(id) == this->result.end()) return 0;
    auto &t = this->result[id];
    return t[flavor];
}

map<string, int> Allocator::count(int phy_id) {
    return this->result[phy_id];
}

int Allocator::count_phy() const {
    return this->result.size();
}

int Allocator::count_vir(const string &flavor) {
    int ret = 0;
    for (auto &it : this->result) {
        ret += it.second[flavor];
    }
    return ret;
}

// used for offline algorithm
// after adding all elements, call this->compute() is a must
void Allocator::add_elem(const string &flavor) {
    this->elems.push_back(flavor);
}

void Allocator::reset() {
    *this = Allocator(cpu, mem, type);
}

template<typename T>
void print_vector(const vector<T> &v) {
    for (const auto &t : v) {
        cout << t << " ";
    }
    cout << endl;
}

// best fit decreasing
bool Allocator::compute() {
    if (this->elems.empty()) return true;
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
                 return a.first > b.first;
             }
         });
    double T = 100;
    double min_T = 1;
    double rate = 0.9999;
    double min_score = 100;
    vector<string> best_elems = this->elems;
    vector<int> idx;
    for (int i = 0; i < this->elems.size(); i++) idx.push_back(i);
    idx.push_back(0); // to avoid out of index error when only on element
    while (T > min_T) {
        map<int, pair<int, int>> cur_resource;
        map<int, map<string, int>> cur_result;
        auto cur_elems = best_elems;
        random_shuffle(idx.begin(), idx.end());
        swap(cur_elems[idx[0]], cur_elems[idx[1]]);
        for (const string &flavor : cur_elems) {
            bool ok = false;
            int cpu_req = CPU[flavorid(flavor)];
            int mem_req = MEM[flavorid(flavor)];
            if (cpu_req > INFO.cpu_lim || mem_req > INFO.mem_lim) {
                continue;
            }
            for (auto &phy : cur_resource) {
                int phy_id = phy.first;
                if (this->type == "CPU") {
                    int &cpu_used = phy.second.first;
                    int &mem_used = phy.second.second;
                    if (cpu_req + cpu_used <= INFO.cpu_lim && mem_req + cpu_used <= INFO.mem_lim) {
                        cpu_used += cpu_req;
                        mem_used += mem_req;
                        cur_result[phy_id][flavor]++;
                        ok = true;
                    }
                } else if (this->type == "MEM") {
                    int &mem_used = phy.second.first;
                    int &cpu_used = phy.second.second;
                    if (cpu_req + cpu_used <= INFO.cpu_lim && mem_req + cpu_used <= INFO.mem_lim) {
                        cpu_used += cpu_req;
                        mem_used += mem_req;
                        cur_result[phy_id][flavor]++;
                        ok = true;
                    }
                }
            }
            if (!ok) {
                pair<int, int> p;
                if (this->type == "CPU") {
                    p.first += cpu_req;
                    p.second += mem_req;
                } else if (this->type == "MEM") {
                    p.first += mem_req;
                    p.second += cpu_req;
                }
                cur_resource[cur_resource.size()] = p;
                cur_result[cur_result.size()][flavor]++;
            }
        }
        const auto p = cur_resource.crbegin();
        double used = p->second.first;
        double use_rate = used / (this->type == "CPU" ? INFO.cpu_lim : INFO.mem_lim);
        double score = cur_resource.size() - 1 + use_rate;
        if (score < min_score || exp((min_score - score) / T) > rand() / RAND_MAX) {
            min_score = score;
            this->result = cur_result;
            this->resource = cur_resource;
            best_elems = cur_elems;
        }
        T *= rate;
    }
    return true;
}

void Allocator::alloc(const string &flavor) {
    int id = flavorid(flavor);
    int cpu_used = CPU[id];
    int mem_used = MEM[id];
    if (cpu_used > this->cpu || mem_used > this->mem) return;
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
        this->alloc(flavor);
    }
}

map<string, int> Allocator::flavor_count() {
    map<string, int> ret;
    for (const auto &phy : this->result) {
        for (const auto &vir : phy.second) {
            ret[vir.first] += vir.second;
        }
    }
    return ret;
};
