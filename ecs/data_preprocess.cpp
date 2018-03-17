#include "data_preprocess.h"
#include "constant.h"
#include <cstdio>
#include <map>
#include <sstream>
#include <vector>
#include <cmath>
#include <iostream>
#include <algorithm>

using namespace std;

RecordSet::RecordSet(const vector<Record> &records) {
    this->records = records;
    for (auto &rec : records) {
        this->by_date[rec.date].push_back(rec);
        this->by_flavor[rec.flavor].push_back(rec);
    }
    // Convert Record to double vector
    for (auto &fr : this->by_flavor) {
        string idx = fr.first;
        vector<Record> &rec = fr.second;
        int bg = 365000, ed = 0;
        int y, m, d;
        for (auto &r : rec) {
            sscanf(r.date.c_str(), "%d-%d-%d", &y, &m, &d);
            int dt = (y - 2000) * 365 + m * 33 + d;
            bg = min(bg, dt);
            ed = max(ed, dt);
        }
        int ss = ed - bg + 1;
        if (ss < 50) ss = 50;
        vector<double> res(ss, 0);
        for(auto &r : rec) {
            sscanf(r.date.c_str(), "%d-%d-%d", &y, &m, &d);
            int dt = ss - (ed - ((y - 2000) * 365 + m * 33 + d)) - 1; // put in proper place
            res[dt] += 1;
        }

        this->data_flavor[idx] = res;
    }
}

int RecordSet::cpu_required(string date) {
    int ret = 0;
    for (auto &rec : this->records) {
        if (rec.date == date) {
            int flavor;
            sscanf(rec.flavor.c_str(), "flavor%d", &flavor);
            ret += CPU[flavor];
        }
    }
    return ret;
}

int RecordSet::mem_required(string date) {
    int ret = 0;
    for (auto &rec : this->records) {
        if (rec.date == date) {
            int flavor;
            sscanf(rec.flavor.c_str(), "flavor%d", &flavor);
            ret += MEM[flavor];
        }
    }
    return ret;
}

SampleByFlavor RecordSet::to_samples(int n) {
    SampleByFlavor ret;
    for (auto &fr : this->data_flavor) {
        string idx = fr.first;
        vector<double> &data = fr.second;
        vector<Sample> tmp;
        for (int i = 0; i < data.size() - n - 1; i++) {
            Sample t;
            for(int j = i; j < i + n; j++) t.X.push_back(data[j]);
            t.y = data[i + n];
            tmp.push_back(t);
        }
        ret[idx] = tmp;
    }
    return ret;
}

vector<double> RecordSet::to_data(int n, string flavor) {
    auto &vec = this->data_flavor[flavor];
    vector<double> ret;
    for (int i = vec.size() - n; i < vec.size(); i++) {
        ret.push_back(vec[i]);
    }
    return ret;
}

Record parse_line(string line) {
    stringstream ss;
    ss << line;
    Record ret;
    ss >> ret.id;
    ss >> ret.flavor;
    ss >> ret.date;
    ss >> ret.time;
    return ret;
}

vector<Record> parse(string content) {
    vector<Record> ret;
    vector<string> text = split(content);
    for (auto line : text) {
        Record record = parse_line(line);
        ret.push_back(record);
    }
    return ret;
}

vector<string> split(string text) {
    vector<string> ret;

    string line;
    stringstream ss;
    for (ss << text; getline(ss, line);) {
        ret.push_back(line);
    }
    return ret;
}
