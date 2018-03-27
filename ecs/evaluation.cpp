//
// Created by 陈斐童 on 21/03/2018.
//

#include "evaluation.h"
#include "data_preprocess.h"
#include "constant.h"

#include <fstream>
#include <sstream>
#include <map>
#include <cmath>

using namespace std;

pair<double, double> evaluate(
        Allocator &alloc,
        Info &meta,
        string test_file
) {
    stringstream ss;
    ifstream file(test_file);
    string line;
    while (getline(file, line)) ss << line << "\n";
    RecordSet testset = RecordSet(parse_records(ss.str()));
    map<string, int> cnt;
    for (int day = meta.start_date; day <= meta.end_date; day++) {
        vector<Record> records = testset.at_date(day);
        for (const auto &rec : records) {
            cnt[rec.flavor]++;
        }
    }

    map<string, int> pred = alloc.flavor_count();
    map<string, pair<int, int>> result; // flavor -> (predict count, real count)
    for (const auto &it : pred) {
        result[it.first].first = it.second;
    }
    for (const auto &it : cnt) {
        result[it.first].second = it.second;
    }

    int phy_cnt = alloc.count_phy();
    double y1y1 = 0, y2y2 = 0, y1y2 = 0;
    double v_mem = 0, h_mem = phy_cnt * meta.mem_lim;
    double v_cpu = 0, h_cpu = phy_cnt * meta.cpu_lim;
    for (const auto &it : result) {
        y1y2 += pow(it.second.first - it.second.second, 2);
        y1y1 += pow(it.second.first, 2);
        y2y2 += pow(it.second.second, 2);

        int id = flavorid(it.first);
        v_mem += MEM[id] * it.second.first;
        v_cpu += CPU[id] * it.second.first;
    }

    y1y2 = sqrt(y1y2 / result.size());
    y1y1 = sqrt(y1y1 / result.size());
    y2y2 = sqrt(y2y2 / result.size());

    double score_mem = (1 - y1y2 / (y1y1 + y2y2)) * (v_mem / h_mem) * 100;
    double score_cpu = (1 - y1y2 / (y1y1 + y2y2)) * (v_cpu / h_cpu) * 100;
    return {score_cpu, score_mem};
}
