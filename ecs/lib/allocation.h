#ifndef ALLOCATION_H_
#define ALLOCATION_H_

#include "constant.h"
#include <map>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

class Allocator {
  public:
    Allocator(int _cpu, int _mem, const string &_type) : cpu(_cpu), mem(_mem), type(_type) {}

    int count(int phy_id, const string &flavor);

    map<string, int> count(int phy_id);

    int count_phy() const;

    int count_vir(const string &flavor);

    // used for offline algorithm
    // after adding all elements, call this->compute() is a must
    void add_elem(const string &flavor);

    void reset();

    // best fit decreasing
    bool compute();

    void alloc(const string &flavor);

    map<string, int> flavor_count();

  private:
    vector<string> elems;
    map<int, pair<int, int>> resource;
    map<int, map<string, int>> result;
    int cpu, mem;
    string type;
};

#endif // ALLOCATION_H_