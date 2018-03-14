#include "data_preprocess.h"
#include "constant.h"
#include <cstdio>
#include <map>
#include <sstream>
#include <vector>
#include <cmath>

using namespace std;

RecordSet::RecordSet(const vector<Record> &records) {
    this->records = records;
    for (auto &rec : records) {
        this->by_date[rec.date].push_back(rec);
        this->by_flavor[rec.flavor].push_back(rec);
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

Normalizer::Normalizer(const vector<Sample> &samples) {
    this->mean.y = 0;
    this->std.y = 0;
    for (int i = 0; i < samples[0].X.size(); i++) {
        this->mean.X.push_back(0);
        this->std.X.push_back(0);
    }
    for (auto &sample : samples) {
        this->mean.y += sample.y / samples.size();
        this->std.y += sample.y * sample.y / samples.size();
        for (int i = 0; i < sample.X.size(); i++) {
            this->mean.X[i] += sample.X[i] / samples.size();
            this->std.X[i] += sample.X[i] * sample.X[i] / samples.size();
        }
    }
    this->std.y = sqrt(this->std.y - this->mean.y * this->mean.y);
    for (int i = 0; i < samples[0].X.size(); i++) {
        this->std.X[i] = sqrt(this->std.X[i] - this->mean.X[i] * this->mean.X[i]);
    }
}

Sample Normalizer::transform(Sample sample) {
    sample.y -= this->mean.y;
    sample.y /= this->std.y;
    for (int i = 0; i < sample.X.size(); i++) {
        sample.X[i] -= this->mean.X[i];
        sample.X[i] /= this->std.X[i];
    }
    return sample;
}

vector<Sample> Normalizer::transform(vector<Sample> samples) {
    vector<Sample> ret;
    for (auto &sample : samples) {
        ret.push_back(this->transform(sample));
    }
    return ret;
}

vector<Sample> RecordSet::to_samples() {
    vector<Sample> samples;
    for (auto &it : this->by_date) {
        Sample sample;
        sample.y = this->cpu_required(it.first);
        int y, m, d;
        sscanf(it.first.c_str(), "%d-%d-%d", &y, &m, &d);
        sample.X.push_back((y - 2000.0) * 356 + m * 33 + d);
        samples.push_back(sample);
    }
    return samples;
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
