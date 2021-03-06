//
// Created by 陈斐童 on 27/03/2018.
//

#include "allocation.h"

#include <iostream>
#include <cmath>
#include <random>

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


// best fit decreasing
bool Allocator::compute() {
    if (this->elems.empty()) return false;
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

    random_device rd;
    default_random_engine rdeg(rd());
    double T = 200;
    double min_T = 1;
    double rate = 0.9999;
    double min_score = 1e200;
    vector<string> best_elems = this->elems;
    map<int, pair<int, int>> total_resource;
    map<int, map<string, int>> total_result;
    double total_score = 1e200;
    while (T > min_T) {
        map<int, pair<int, int>> cur_resource;
        map<int, map<string, int>> cur_result;
        auto cur_elems = best_elems;
        int s = max(0, static_cast<int>(this->elems.size()) - 1);
        uniform_int_distribution<int> unidist(0, s);
        for (int i = 0; i < T/10 + 1; i++) {
            int a = unidist(rdeg), b = unidist(rdeg);
            swap(cur_elems[a], cur_elems[b]);
        }
        for (const string &flavor : cur_elems) {
            bool ok = false;
            int cpu_req = CPU[flavorid(flavor)];
            int mem_req = MEM[flavorid(flavor)];
            if (cpu_req > this->cpu || mem_req > this->mem) {
                continue;
            }
            for (auto &phy : cur_resource) {
                int &cpu_used = phy.second.first;
                int &mem_used = phy.second.second;
                if (cpu_req + cpu_used <= this->cpu && mem_req + mem_used <= this->mem) {
                    cpu_used += cpu_req;
                    mem_used += mem_req;
                    cur_result[phy.first][flavor]++;
                    ok = true;
                    break;
                }
            }
            if (!ok) {
                pair<int, int> p{cpu_req, mem_req};
                cur_resource[cur_resource.size()] = p;
                cur_result[cur_result.size()][flavor]++;
            }
        }
        double score = 0;
        for (const auto &p : cur_resource) {
            if (this->type == "CPU") {
                score += 1 - p.second.first * 1.0 / this->cpu;
            } else if (this->type == "MEM") {
                score += 1 - p.second.second * 1.0 / this->mem;
            }
        }
        if (score < min_score || exp((min_score - score) / T) > rand() / RAND_MAX) {
            min_score = score;
            this->result = cur_result;
            this->resource = cur_resource;
            best_elems = cur_elems;
        }
        if (score < total_score) {
            total_score = score;
            total_result = cur_result;
            total_resource = cur_resource;
        }

        T *= rate;
    }
    if (this->result.size() > total_result.size()) {
        this->result = total_result;
        this->resource = total_resource;
    }
    return true;
}

void Allocator::postprocess() {
    auto p = this->resource.rbegin();
    double use_rate = 0;
    if (this->type == "CPU") {
        use_rate = p->second.first * 1.0 / this->cpu;
    } else if (this->type == "MEM") {
        use_rate = p->second.second * 1.0 / this->mem;
    }
    if (use_rate < 0.4 && this->resource.size() > 1) {
        int id = this->resource.size();
        this->resource.erase(id);
        this->result.erase(id);
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
