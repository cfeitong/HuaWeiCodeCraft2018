//
// Created by 陈斐童 on 27/03/2018.
//

#include "allocation.h"
#include <iostream>
#include <algorithm>
#include <cmath>

int Allocator::count(int phy_id, string flavor) {
    return this->result[phy_id-1][flavor];
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

template <typename T>
void print_vector(const vector<T> &v) {
    for (const auto &t : v) {
        cout << t << " ";
    }
    cout << endl;
}

// best fit decreasing
void Allocator::compute() {
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
    map<int, pair<int, int>> best_resource;
    map<int, map<string, int>> best_result;
    vector<string> best_elems = this->elems;
    vector<int> idx;
    for (int i = 0; i < this->elems.size(); i++) idx.push_back(i);
    while (T > min_T) {
        auto cur_elems = best_elems;
        random_shuffle(idx.begin(), idx.end());
        swap(cur_elems[idx[0]], cur_elems[idx[1]]);
        double score = 0;
        if (score < min_score || exp((min_score - score) / T) > rand() / RAND_MAX) {
            min_score = score;
            this->result = best_result;
            this->resource = best_resource;
        }
        T *= rate;
    }

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
