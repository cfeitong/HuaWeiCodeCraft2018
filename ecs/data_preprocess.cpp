#include "data_preprocess.h"
#include "lib_time.h"
#include "constant.h"
#include <cstdio>
#include <map>
#include <sstream>
#include <utility>
#include <vector>
#include <cmath>
#include <iostream>
#include <algorithm>

using namespace std;

ostream &operator<<(ostream &os, const Sample &obj) {
    os << "{";
    for (auto it : obj.X) {
        os << it << ",";
    }
    os << "} -> " << obj.y;
    return os;
}

RecordSet::RecordSet(const vector<Record> &records) {
    this->records = records;
    for (auto &rec : records) {
        this->by_flavor[rec.flavor].push_back(rec);
        this->by_date[rec.time].push_back(rec);
    }
    // Convert Record to double vector
    for (auto &fr : this->by_flavor) {
        string idx = fr.first;
        vector<Record> &rec = fr.second;
        int bg = 365000, ed = 0;
        for (auto &r : rec) {
            int dt = r.time;
            bg = min(bg, dt);
            ed = max(ed, dt);
        }
        int ss = ed - bg + 1;
        vector<double> res(ss, 0);
        for (auto &r : rec) {
            int dt = ss - (ed - r.time) - 1; // put in proper place
            res[dt] += 1;
        }
        while (res.size() < 50) {
            res.insert(res.end(), res.begin(), res.end());
        }
        vector<double> tmp(res.end(), res.end());

        this->data_flavor[idx] = res;
    }
}

map<string, vector<Sample>> RecordSet::to_samples() {
    map<string, vector<Sample>> ret;
    int n = INFO.block_count;
    for (auto &f : INFO.targets) {
        const auto &data = this->to_data(f);
        vector<Sample> samples;
        for (int i = 0; i < data.size() - n; i++) {
            Sample sample{{}, 0};
            for (int j = i; j < i + n; j++) {
                sample.X.push_back(data[j]);
            }
            sample.y = data[i + n];
            samples.push_back(sample);
        }
        ret[f] = samples;
    }
    return ret;
}

vector<double> RecordSet::to_data(const string &flavor) {
    int days = INFO.days;
    auto &vec = this->data_flavor[flavor];
    vector<double> ret;
    for (int i = (int) vec.size(); i >= 0; i -= days) {
        double s = 0;
        for (int j = i - 1; j >= i - days; j--) {
            if (j >= 0) {
                s += vec[j];
            } else {
                s += vec[0];
            }
        }
        ret.push_back(s);
    }
    reverse(ret.begin(), ret.end());
    return ret;
}

vector<double> RecordSet::get_data(const string &flavor) {
    return this->data_flavor[flavor];
}

vector<Record> RecordSet::at_date(int day) {
    return this->by_date[day];
}

Record parse_line(string line) {
    stringstream ss;
    ss << line;
    Record ret;
    ss >> ret.id;
    ss >> ret.flavor;
    string date;
    int y, m, d;
    ss >> date;
    sscanf(date.c_str(), "%d-%d-%d", &y, &m, &d);
    ret.time = to_days(y, m, d);
    return ret;
}

vector<Record> parse_records(string content) {
    vector<Record> ret;
    vector<string> text = split(move(content));
    for (const auto &line : text) {
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
