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
        if (ss < 50) ss = 50;
        vector<double> res(ss, 0);
        for(auto &r : rec) {
            int dt = ss - (ed - r.time) - 1; // put in proper place
            res[dt] += 1;
        }

        this->data_flavor[idx] = res;
    }
}

SampleByFlavor RecordSet::to_samples(int n, int days) {
    SampleByFlavor ret;
    for (auto &fr : this->data_flavor) {
        string idx = fr.first;
        vector<double> &data = fr.second;
        vector<Sample> tmp;
        for (int i = 0; i < data.size() - (n + 1) * days - 1; i++) {
            Sample t;
            for (int j = 0; j < n; j++) {
                double s = 0;
                for(int k = i + j * days; k < i + j * days + days; k++) {
                    s += data[k];
                }
                t.X.push_back(s);
            }
            t.y = 0;
            for (int j = i + n * days; j < i + (n + 1) * days; j++) t.y += data[j];
            tmp.push_back(t);
        }
        ret[idx] = tmp;
    }
    return ret;
}

vector<double> RecordSet::to_data(int n, int days, string flavor) {
    auto &vec = this->data_flavor[flavor];
    vector<double> ret;
    for (int i = 0; i < n; i++) {
        double s = 0;
        for (int j = vec.size() - (n - i) * days; j < vec.size() - (n - i - 1) * days; j++) s += vec[j];
        ret.push_back(s);
    }
    return ret;
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
    string date; int y, m, d;
    ss >> date; sscanf(date.c_str(), "%d-%d-%d", &y, &m, &d);
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
